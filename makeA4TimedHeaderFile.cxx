#include "RawAnitaHeader.h" 
#include <vector>
#include <stdlib.h>
#include "TMath.h" 
#include <stdio.h>
#include "TTree.h" 
#include "TFile.h" 


/** Hastily written crappy code for A-4 timing correction */ 


void meanRmsDifference(int N, const double * a, const double * b, double * pmean, double * prms) 
{
  double sum = 0; 
  double sum2 = 0; 

  for (int i = 0; i < N; i++) 
  {
    double diff = a[i]-b[i]; 
    sum +=diff; 
    sum2 += diff*diff; 
  }

  sum/=N; 

  if (pmean) *pmean = sum; 
  if (prms) *prms = sqrt(sum2/N-sum*sum); 
}




double alignmentFraction(int N, const double * trigger_times, int delta, int Nttt, double * ttt, double eps = 0.005) 
{

  if (Nttt == 0) return 0; 

  //start by bracketing the ttts 
  int ttt_i = 0; 

  while ( ttt[ttt_i] < trigger_times[0]-delta ) ttt_i++; 

  int min_ttt = ttt_i; 

  ttt_i = Nttt-1; 

  while (ttt[ttt_i] > trigger_times[N-1]-delta) ttt_i--; 

  int max_ttt = ttt_i; 

  int max_matches = max_ttt - min_ttt + 1; 

  //now search for each ttt in trigger_times 


  printf("max_matches: %d\n", max_matches); 
  int nmatches = 0; 
  for (int i = min_ttt; i <= max_ttt;i++)
  {
    int idx = TMath::BinarySearch(N, trigger_times, ttt[i]+delta); 

    if ( fabs(trigger_times[idx]-delta -ttt[i]) < eps)
    {
      nmatches++; 
    }
    else if (idx < N-1 && fabs(trigger_times[idx+1] -delta- ttt[i]) < eps) 
    {
      nmatches++; 
    }
  }


  return double(nmatches) / max_matches; 
}



void makeTimedHeaderTree(int run) 
{

  printf("Processing run %d\n",run); 

  RawAnitaHeader * raw  = 0; 
  TFile fhead(TString::Format("%s/run%d/headFile%d.root",getenv("ANITA_ROOT_DATA"), run, run)); 
  TTree * headTree = (TTree*) fhead.Get("headTree"); 
  headTree->SetBranchAddress("header",&raw); 

  int secOffset = headTree->GetMinimum("payloadTime"); 

  TFile ftimed(TString::Format("%s/run%d/timedHeadFile%d.root",getenv("ANITA_ROOT_DATA"), run, run), "RECREATE"); 
  TTree * timedTree = new TTree ("headTree","Timed  Head Tree"); 
  timedTree->Branch("header",&raw); 

  TFile fttt(TString::Format("%s/run%d/tttFile%d.root", getenv("ANITA_ROOT_DATA"), run, run)); 
  TTree * tttTree = (TTree*) fttt.Get("tttTree"); 

  int Nttt = 0; 
  double * ttt = 0; 
  //construct ttt ttree, if we have any


  if (tttTree) 
  {
    tttTree->Draw(TString::Format("unixTimeGps-3-%d+subTimeGps*1e-7",secOffset)); //GPS offset of 14 already applied . GPS offset should be 17 for A4 flight
    Nttt = tttTree->GetEntries(); 
    ttt = tttTree->GetV1(); 
    printf("nttt: %d\n", Nttt); 
  }

  std::vector<int> epochs; 
  epochs.push_back(0); 
  int nepochs = 1; 

 headTree->Draw(TString::Format("triggerTime-%d + 1e-9*triggerTimeNs:payloadTime-%d+1e-6*payloadTimeUs:trigType&0xf:ppsNum",secOffset,secOffset),"","goff"); 

 double * trigger_time= headTree->GetV1(); 
 double * payload_time = headTree->GetV2(); 
 double * trigType = headTree->GetV3(); 
 double * pps= headTree->GetV4(); 

 int N = headTree->GetEntries(); 
 double* corrected_times = new double[N]; 
 int *good_time_flags = new int[N]; 
 int last = 0; 

 /** build epochs 
  *
  **/ 
 for (int i = 0; i < N; i++) 
 {
    if (pps[i] < last) 
    {
      epochs.push_back(i); 
      nepochs++; 
    }
    last = pps[i]; 
  }
  epochs.push_back(headTree->GetEntries()); 


 for (int epoch = 0; epoch < nepochs; epoch++)
 {

   double mean_diff,rms_diff; 
   int start = epochs[epoch]; 
   int end = epochs[epoch+1]; 
   meanRmsDifference(end-start, trigger_time + start, payload_time + start, &mean_diff, &rms_diff); 

   printf("%g %g\n",mean_diff, rms_diff); 

   if (rms_diff > 1) 
   {
     // that means we don't have a pps for the whole thing, so let's just set according to payload time when no pps. We will recalculate the offset at each pps trigger. 
     // note that we don't yet detect cases where there is a combination of ppsNum incrementing and not
     
     //first, let's find the first pps pulse. For some reason, this is 4 when ppsNum is gone... I think.  

     double offset = 0; 
     double * pps = std::find(trigType+start, trigType+end, 4); 
     if (pps >= trigType+end) 
     {
       offset = -0.2; //close enough!
     }
     else 
     {
       int idx = (int) (pps - (trigType+start)); 
       offset = 1+int(payload_time[idx]) - payload_time[idx]; 
     }

     printf("start offset: %g\n",offset); 
     for (int i = start; i < end; i++) 
     {

       if (trigType[i] == 4) 
       {
         offset = 1-payload_time[i] + int(payload_time[i]); 
         printf("new offset: %g\n",offset); 
       }

       corrected_times[i] = payload_time[i] + offset; 
       good_time_flags[i] = -1; 
     }

   }
   else
   {

     int delta = round(mean_diff); 


     if (delta != 0 && delta != -1) 
     {
       fprintf(stderr,"!!!! !!!! delta expected to be 0 or -1");  
       return; 
     }

     double match_0 = alignmentFraction(end-start, trigger_time+start,0, Nttt, ttt); 
     double match_n1 = alignmentFraction(end-start, trigger_time+start,-1, Nttt, ttt); 

     if  (  (delta == 0 && match_n1 > match_0 ) || (delta == -1 && match_0 > match_n1))
     {
       fprintf(stderr, "!!!! alignment disagrees with paylaod time!!!!\n"); 

     }

     printf("Chose delta=%d for epoch %d. alignment is %g (other alignment is %g)\n", delta, epoch, delta == 0 ? match_0 : match_n1, delta == 0 ? match_n1 : match_0); 


     //offset correction
     for (int i = start; i < end; i++) 
     {
       corrected_times[i] = trigger_time[i] - delta; 

       //final glitch detection
       if (fabs(corrected_times[i] - payload_time[i]) > 0.5 )
       {
         corrected_times[i] -= round(corrected_times[i] - payload_time[i]); 
         good_time_flags[i] = -3; 
       }
       else
       {
         good_time_flags[i] = 1; 
       }
     }
   }
 }


 for ( int i = 0; i < N; i++) 
 {
   headTree->GetEntry(i); 
   ftimed.cd(); 
   raw->triggerTime = int(corrected_times[i])+secOffset; 
   raw->triggerTimeNs = 1e9*(corrected_times[i] - int(corrected_times[i])); 
   raw->goodTimeFlag = good_time_flags[i]; 
   timedTree->Fill(); 
 }

 ftimed.cd(); 
 timedTree->Write(); 

}



int main (int nargs, char ** args) 
{


  for (int i = 1; i <nargs; i++) 
  {

    makeTimedHeaderTree(atoi(args[i])); 

  }



}


