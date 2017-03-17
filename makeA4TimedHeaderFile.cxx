#include "RawAnitaHeader.h" 
#include <vector>
#include "TimedAnitaHeader.h" 
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

  if (Nttt == 0 || N == 0) return -1; 

  //start by bracketing the ttts 
  int ttt_i = 0; 

  while ( ttt[ttt_i] < trigger_times[0]-delta ) ttt_i++; 

  int min_ttt = ttt_i; 

  ttt_i = Nttt-1; 

  while (ttt[ttt_i] > trigger_times[N-1]-delta)
  {
    ttt_i--; 
    if (ttt_i < 0) return -1; 
  }

  int max_ttt = ttt_i; 

  int max_matches = max_ttt - min_ttt + 1; 

  //now search for each ttt in trigger_times 


  printf("max_matches: %d\n", max_matches); 
  if (!max_matches) return -1; 
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
    else
    {
//      printf("  No good alignment for delta=%d, ttt=%g (%g,%g)\n", delta, ttt[i], trigger_times[idx], idx < N-1 ? trigger_times[idx+1] : -1); 
    }
  }


  return double(nmatches) / max_matches; 
}



void makeTimedHeaderTree(int run) 
{

  printf("Processing run %d\n",run); 

  RawAnitaHeader * raw  = 0; 
  TimedAnitaHeader * timed  = new TimedAnitaHeader; 
  TFile fhead(TString::Format("%s/run%d/headFile%d.root",getenv("ANITA_ROOT_DATA"), run, run)); 
  TTree * headTree = (TTree*) fhead.Get("headTree"); 
  headTree->SetBranchAddress("header",&raw); 

  unsigned secOffset = headTree->GetMinimum("payloadTime"); 

  TFile ftimed(TString::Format("%s/run%d/timedHeadFile%d.root",getenv("ANITA_ROOT_DATA"), run, run), "RECREATE"); 
  TTree * timedTree = new TTree ("headTree","Timed  Head Tree"); 
  timedTree->Branch("header",&timed); 

  TFile fttt(TString::Format("%s/run%d/tttFile%d.root", getenv("ANITA_ROOT_DATA"), run, run)); 
  TTree * tttTree = (TTree*) fttt.Get("tttTree"); 

  int Nttt = 0; 
  double * ttt = 0; 
  //construct ttt ttree, if we have any


  if (tttTree) 
  {
    tttTree->Draw(TString::Format("unixTimeGps-3-%u+subTimeGps*1e-7",secOffset),"","goff"); //GPS offset of 14 already applied . GPS offset should be 17 for A4 flight
    Nttt = tttTree->GetEntries(); 
    ttt = tttTree->GetV1(); 
    std::sort(ttt,ttt+Nttt); 
    printf("nttt: %d\n", Nttt); 
  }

  std::vector<int> epochs; 
  epochs.push_back(0); 
  int nepochs = 1; 

 headTree->Draw(TString::Format("triggerTime-%u + 1e-9*triggerTimeNs:payloadTime-%u+1e-6*payloadTimeUs:trigType&0xf:ppsNum:trigTime:rawc3poNum:goodTimeFlag",secOffset,secOffset),"","paragoff"); 

 double * trigger_time= headTree->GetV1(); 
 double * payload_time = headTree->GetV2(); 
 double * trigType = headTree->GetV3(); 
 double * pps= headTree->GetV4(); 
 double * trig = headTree->GetVal(4); 
 double * c3po = headTree->GetVal(5); 
 double * old_good_time_flag = headTree->GetVal(6); 

 int N = headTree->GetEntries(); 
 double* corrected_times = new double[N]; 
 int *good_time_flags = new int[N]; 


 int last_pps = pps[0]; 
 double last_pps_payload_time = payload_time[0]; 
 bool back_to_normal = false;
 
 /** build epochs 
  *
  **/ 

 bool stalled = pps[0] ==0; 
 for (int i = 1; i < N; i++) 
 {
    
    back_to_normal = (c3po[i-1] < 200e6 || c3po[i-1] > 300e6) && (c3po[i] > 200e6 && c3po[i] < 300e6) ; 


    if (pps[i] < pps[i-1]   // pps smaller than before
       || payload_time[i] > payload_time[i-1]+2   //big gap in payload time 
       || (pps[i] == last_pps && trig[i] > 600e6 && !stalled)   //pps stalled updating
       || (pps[i] > last_pps && stalled) // pps resumed updating
       || back_to_normal //after c3poNum glitch
       )
    {

      printf("New epoch at %d\n",i); 
      epochs.push_back(i); 
      nepochs++; 
    }

    if (pps[i] > last_pps)
    {
      if (stalled) printf("unstalled? %g %g\n", pps[i], payload_time[i] ); 
      stalled = false; 
      last_pps = pps[i]; 
      last_pps_payload_time = payload_time[i]; 
    }
    else if (!stalled && pps[i] == last_pps && trig[i] > 600e6)
    {
      if (!stalled) printf("stalled? %g %g %g\n", pps[i], payload_time[i], last_pps_payload_time ); 
      stalled = true; 
    }

  }
  epochs.push_back(headTree->GetEntries()); 

 double *aligned = new double[nepochs]; 

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
     //first, let's find the first pps pulse. For some reason, this is 4 when ppsNum is gone... I think.  

     double offset = 0.2; 
     double * pps = std::find(trigType+start, trigType+end, 4); 
     if (pps >= trigType+end) 
     {
       offset = 0.2; //close enough!
     }
     else 
     {
       int idx = (int) (pps - (trigType+start)); 
       double new_offset = 1+int(payload_time[idx]) - payload_time[idx]; 
       
       //only update if it's reasonable 
       if (new_offset > 0.1 && new_offset < 0.3) offset=new_offset; 
     }

     printf("start offset: %g\n",offset); 
     for (int i = start; i < end; i++) 
     {

       if (trigType[i] == 4) 
       {
         double new_offset = 1-payload_time[i] + int(payload_time[i]); 
         if (new_offset > 0.1 && new_offset < 0.3) offset=new_offset; 
         printf("new offset: %g\n",offset); 
       }

       corrected_times[i] = payload_time[i] + offset; 
       good_time_flags[i] = 0; 
     }

     aligned[epoch] = alignmentFraction(end-start, corrected_times+start,0,Nttt,ttt); 
   }
   else
   {

     int delta = round(mean_diff); 
     double offset = 0.2; 



     double match[5]; 

     for (int i = -2; i<=2; i++) 
     {
       match[2+i] = alignmentFraction(end-start, trigger_time+start,delta+i, Nttt, ttt); 
       printf("Delta: %d, Alignment: %g\n", delta+i, match[2+i]); 
     }
       

     printf("Chose delta=%d for epoch %d. alignment is %g\n", delta, epoch, match[2]); 

     //offset correction
     for (int i = start; i < end; i++) 
     {
       corrected_times[i] = trigger_time[i] - delta; 

       //final glitch detection
       if (fabs(corrected_times[i] -offset- payload_time[i]) > 0.75 )
       {
         corrected_times[i] -= round(corrected_times[i]-offset - payload_time[i]); 
         good_time_flags[i] = -3; 
       }
       else
       {
         good_time_flags[i] = old_good_time_flag[i]; 
       }
     }

     //this is the real calculation
     aligned[epoch] = alignmentFraction(end-start, corrected_times+start,0,Nttt,ttt); 
   }
 }


 int j = 0; //epoch index 
 for ( int i = 0; i < N; i++) 
 {
   headTree->GetEntry(i); 
   ftimed.cd(); 
   if (i >= epochs[j+1]) j++; 
   new (timed) TimedAnitaHeader(*raw, unsigned(corrected_times[i]) + secOffset, 1e9 * (corrected_times[i] - int(corrected_times[i])), aligned[j]); 
   timed->goodTimeFlag = good_time_flags[i]; 
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


