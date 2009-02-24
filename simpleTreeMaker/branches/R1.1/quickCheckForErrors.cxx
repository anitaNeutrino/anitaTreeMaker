#include <cstdio>
#include <fstream>
#include <iostream>
#include <zlib.h>
#include <libgen.h>
#include <errno.h>
   


#include "TTree.h"
#include "TChain.h"
#include "TFile.h"
#include "TSystem.h"
#include "TH1.h"

#define HACK_FOR_ROOT

#include "RawAnitaHeader.h"
#include "RawAnitaEvent.h"
#include "SurfHk.h"
#include "AcqdStart.h"
#include "AnitaGeomTool.h"
#include "UsefulAnitaEvent.h"


#define BASE_LOG_DIR "/home/rjn/anita/errorLogs/neo"
using namespace std;

int getEventEntry();
void quickCheckForErrors();
void Zero(int *anarray,int n);
void MinusOnes(int *anarray, int n);
void MinusOnes(double *anarray, int n);
void lookForWaveformFunniness();

//Some global variables
UInt_t fCurrentRun;
TFile *fHeadFile;
TFile *fEventFile;
TFile *fSurfHkFile;
TFile *fAcqdFile;
TTree *fHeadTree;
TChain *fEventTree;
TTree *fSurfHkTree;
TTree *fAcqTree;
RawAnitaHeader *fHeadPtr;
RawAnitaEvent *fRawEventPtr;
SurfHk   *fSurfPtr;
AcqdStart *fAcqPtr;
UsefulAnitaEvent *fUsefulEventPtr=0;

Long64_t fSurfHkEntry=0;
Long64_t fEventEntry=0;


char fCurrentBaseDir[FILENAME_MAX];

int main(int argc, char **argv) {
  if(argc<3) {
    std::cout << "Usage: " << basename(argv[0]) << " <base dir> <run number>" << std::endl;
    return -1;
  }
  strcpy(fCurrentBaseDir,argv[1]);
  fCurrentRun=atoi(argv[2]);

  quickCheckForErrors();
  lookForWaveformFunniness();
  return 0;
}


void loadAcqdTree()
{
  char acqname[FILENAME_MAX];
  sprintf(acqname,"%s/run%d/auxFile%d.root",fCurrentBaseDir,fCurrentRun,fCurrentRun);
  fAcqdFile = new TFile(acqname);
  if(!fAcqdFile) {
    cout << "Couldn't open: " << acqname << "\n";
    return;
  }
  
  fAcqTree = (TTree*) fAcqdFile->Get("acqdStartTree");
  
  if(!fAcqTree) {
    cout << "Couldn't get AcqdTree from " << acqname << endl;
   return;
  }
  
  fAcqTree->SetBranchAddress("acqd",&fAcqPtr);
  
}


void loadSurfTree()
{
 char surfName[FILENAME_MAX];
 sprintf(surfName,"%s/run%d/surfHkFile%d.root",fCurrentBaseDir,
	 fCurrentRun,fCurrentRun);
 fSurfHkFile = new TFile(surfName);
 if(!fSurfHkFile) {
   cout << "Couldn't open: " << surfName << "\n";
   return;
      }
 fSurfHkTree = (TTree*) fSurfHkFile->Get("surfHkTree");
 if(!fSurfHkTree) {
   cout << "Couldn't get surfHkTree from " << surfName << endl;
   return;
 }
 fSurfHkTree->SetBranchAddress("surf",&fSurfPtr);
 fSurfHkEntry=0;


}


void loadEventTree()
{       
  char eventName[FILENAME_MAX];
  char headerName[FILENAME_MAX];
  sprintf(eventName,"%s/run%d/eventFile%d*.root",fCurrentBaseDir,fCurrentRun,fCurrentRun);
  sprintf(headerName,"%s/run%d/headFile%d.root",fCurrentBaseDir,fCurrentRun,fCurrentRun);
  fEventTree = new TChain("eventTree");
  fEventTree->Add(eventName);
  fEventTree->SetBranchAddress("event",&fRawEventPtr);

      
  fHeadFile = new TFile(headerName);
  if(!fHeadFile) {
    cout << "Couldn't open: " << headerName << "\n";
    return;
  }
  fHeadTree = (TTree*) fHeadFile->Get("headTree");
  if(!fHeadTree) {
    cout << "Couldn't get headTree from " << headerName << endl;
    return;
  }
  fHeadTree->SetBranchAddress("header",&fHeadPtr);
  fEventEntry=0;
  fHeadTree->BuildIndex("eventNumber");
  //      fHeadIndex = (TTreeIndex*) fHeadTree->GetTreeIndex();
  //  std::cerr << fEventTree << "\t" << fHeadTree << "\n";
  //  std::cerr << fHeadTree->GetEntries() << "\t"
  //	    << fEventTree->GetEntries() << "\n";
}

int getEventEntry()
{

  if(!fEventTree) loadEventTree(); 
   if(fEventEntry<fEventTree->GetEntries())
      fEventTree->GetEntry(fEventEntry);
   else {
      std::cout << "No more entries in event tree" << endl;
      return -1;
   }
            
   if(fEventEntry<fHeadTree->GetEntries())
      fHeadTree->GetEntry(fEventEntry);
   else {
      std::cout << "No more entries in header tree" << endl;
      return -1;
   }
   if(fUsefulEventPtr)
     delete fUsefulEventPtr;
   fUsefulEventPtr = new UsefulAnitaEvent(fRawEventPtr,WaveCalType::kVoltageTime);  
   //fUsefulEventPtr = new UsefulAnitaEvent(fRawEventPtr,WaveCalType::kNoCalib); 
   //Need to make configurable at some point
   //This will also need to be modifed to make realEvent accessible outside here
   return 0;
}

 
void quickCheckForErrors() {

  loadEventTree();
  loadSurfTree();
  loadAcqdTree();

  char filename[150];
  char histname[180];
  char histtitle[180];

  sprintf(filename,"%s/errs_%d.txt",BASE_LOG_DIR,fCurrentRun);
  ofstream ferrs(filename);

  int mytriggerbits[4]={0};


  //plot RF triggers by priority number
  TH1F *htriggerTimeNsRF_priority[10];
  for (int ctr=0;ctr<10;ctr++){
    sprintf(histname,"htriggertimeNsRF_priority_%d",ctr);
    sprintf(histtitle,"trigger time in ns for RF triggers, for events with priority %d", ctr+1);
    htriggerTimeNsRF_priority[ctr]=new TH1F(histname,histtitle,200,0,1e9);
    htriggerTimeNsRF_priority[ctr]->SetXTitle("(triggerTimeNs)");
    // else htriggerTimeNs[ctr]->SetXTitle("log(triggerTimeNs)");
    htriggerTimeNsRF_priority[ctr]->SetYTitle("Number of Events"); 
  }

  //c3po's that are not good
  int badC3poCount=0;



  int chipdiff; //difference between this and previous
  int labchip_previous; //lab chip id for previous event
  int labchip_this; //lab chip id for this event

  //  int reserveddiff;
  int reserved_previous;
  int reserved_this;

  int ppsNum_previous;
  int ppsNum_this; //

  int surf0evnum_previous;
  int surf0evnum_this;

  int surf1evnum_previous;
  int surf1evnum_this;

  int trigNum_previous;
  int trigNum_this;

  int priority_this;

  UInt_t eventNumber_previous;
  //int reservedlabdiff;

  int tTdiff;
  int tT_previous;
  int tT_this;//trigger time in nanoseconds
  
  int startevent=0;
  int endevent=fEventTree->GetEntries();

  int errflag=0;//number of errors in each event
  int surfidmismatch[ACTIVE_SURFS];
  int surfturfidmismatch[ACTIVE_SURFS];
  int labchipmismatch[ACTIVE_SURFS][NUM_CHAN];
  int chipsequenceerr=-1; //gets set to event number if there is an error
  int chipreservederr=-1;
  int resetppsnum=-1;
  int resettrignum=-1;
  int mismatched_tree_sizes=-1;
  int sequential_event_numbers=-1;

  int sync_slips_surf_to_turf=0; //total number of sync slips in this run
  int sync_slips_surf_to_turf_0_surf_num=0; //total number of sync slips in this run
  int sync_slips_surf_to_surf=0;//total number of synch slips in this run

  long int n_ground_pulser[9];// 9 priorities
  //  int doublePulseFlag=1;//set if we are double pulseing
  //  int last_event_was_ground_pulser=0;//flag that tells you if the last event was indeed the ground pulser
  //  UInt_t groundPulserTime=650000000; //look on 650 ms
  //  UInt_t groundPulserWindow=10000;//10 microseconds
  //  int efficiency_numerator=0;
  //  int avg_ctr=0;
  //  int efficiency_denominator=0;
  //  double efficiency_of_double_pulses=0;
  //  double average_doublePulse_triggerdiff=0;
  
  for (int ctr1=0;ctr1<9;ctr1++){
    n_ground_pulser[ctr1]=0;
  }

  //double overflows[ACTIVE_SURFS][SCALERS_PER_SURF];
  int triggerTypeFunny=-1;

  int print_every_percent=10;
  int print_every=(int)((double)fEventTree->GetEntries()/(double)print_every_percent);

  cout << "There are " << fEventTree->GetEntries() << " events in the Event Tree.\n";

  //get maximum and minimum to set histogram boundaries.

  
  for (int i=startevent;i<endevent;i++) {
    if ((i-startevent)%print_every==0)
      cout << "Processed " << (i-startevent)/print_every*10. << " percent of the events.\n";
    errflag=0;
    MinusOnes(surfidmismatch,ACTIVE_SURFS); 
    MinusOnes(surfturfidmismatch,ACTIVE_SURFS);
    chipsequenceerr=-1;
    chipreservederr=-1;
    resetppsnum=-1;
    resettrignum=-1;
    mismatched_tree_sizes=-1;

    fEventTree->GetEvent(i);
    fHeadTree->GetEntry(i);
    
    //  cout << "entry, event numbers, entrywithindex are " << i << " " << fRawEventPtr->eventNumber << " " 
    //        << fHeadPtr->eventNumber << " " << fHeadTree->GetEntryWithIndex(fRawEventPtr->eventNumber) << "\n";

    if (fHeadTree->GetEntryWithIndex(fRawEventPtr->eventNumber)==-1){
      mismatched_tree_sizes=i;
      i=endevent;
      continue;
    }
    
    if (fRawEventPtr->eventNumber!=eventNumber_previous+1 && i!=0){ 
      errflag++;
      sequential_event_numbers=i;
    }

    reserved_this=fHeadPtr->reserved[0]&0xf;
    surf0evnum_this=fHeadPtr->otherFlag&0xf;
    surf1evnum_this=(fHeadPtr->otherFlag&0xf0)>>4;

    trigNum_this=fHeadPtr->trigNum;

    tT_this=fHeadPtr->triggerTimeNs;
    ppsNum_this=fHeadPtr->ppsNum;
    

    // hc3po->Fill(fHeadPtr->c3poNum-125.E6);
    if(fHeadPtr->c3poNum<124e6 || fHeadPtr->c3poNum>126e6) {
      badC3poCount++;
      ferrs << "Bad c3poNum: " << fHeadPtr->eventNumber << "\t" << fHeadPtr->c3poNum << "\n";
    }


    priority_this=(fHeadPtr->priority&0xf0)/16;
    
    if (i!=startevent) {
      tTdiff=tT_this-tT_previous;
      
      if (tTdiff<0)
	tTdiff+=1000000000;//add a second if the previous event happened at the end of a second
  
      /* //reserveddiff is actually reserved quotient. which should = 2.  
      if (reserved_this==1) //accounts for wrap-around case (8->1)
	reserveddiff=16*reserved_this/reserved_previous;
      else
	reserveddiff=reserved_this/reserved_previous;
            
      hdreserved_eventtoevent->Fill(reserveddiff);
      */
      
      /* hdtriggerTimeNs->Fill(log10(tTdiff));
      if (log10(tTdiff)<4.){//close events (for peter)
	hdtriggerTimeNs_close->Fill(tTdiff);
	hdtriggerTimeNs_close_zoom->Fill(tTdiff);
      }
      */
      
    } // end if it's not the first event

    for (int surf=0;surf<ACTIVE_SURFS;surf++) {
      // hsurfiddiff[surf]->Fill(fRawEventPtr->surfEventId[(surf+1)%ACTIVE_SURFS]-fRawEventPtr->surfEventId[surf]);
      if (fRawEventPtr->surfEventId[(surf+1)%ACTIVE_SURFS]!=fRawEventPtr->surfEventId[surf]) {//check for nonequal surfeventids
	errflag++;
	surfidmismatch[surf]=i;
      }

      //hturfsurfdiff[surf]->Fill(fRawEventPtr->surfEventId[surf]-fHeadPtr->turfEventId);
      if (fRawEventPtr->surfEventId[surf]!=fHeadPtr->turfEventId) {//check for nonequal turf and surf event ids
	errflag++;
	surfturfidmismatch[surf]=i; //set error flag equal to index
      }


      for (int chan=0;chan<NUM_CHAN;chan++) {
	MinusOnes(labchipmismatch[surf],NUM_CHAN);
	
	int chanIndex=AnitaGeomTool::getChanIndex(surf,chan);
	//	cout << "surf, labChip is " << surf << " " << labChip << "\n";
	int chanIndex1=(chanIndex+1)%(ACTIVE_SURFS*NUM_CHAN);
	int chanIndex2=chanIndex;
	
	//hsurflabchipdiff[chanIndex]->Fill(getLabChip(fRawEventPtr,chanIndex1)-getLabChip(fRawEventPtr,chanIndex2));

	if (fRawEventPtr->getLabChip(chanIndex1)!= fRawEventPtr->getLabChip(chanIndex2)) {
	  errflag++;
	  labchipmismatch[surf][chan]=i;
	  
	}

	if (chanIndex==0) {//only do this once per surf
	  labchip_this=fRawEventPtr->getLabChip(chanIndex);
	  
	  if (i!=startevent) {
	    chipdiff=labchip_this-labchip_previous;

	    if (labchip_this==0)
	      chipdiff+=4;


	    //hdchip_eventtoevent->Fill(chipdiff);

	    if (ppsNum_this<7 && ppsNum_previous>=7 && ppsNum_previous!=pow(2,16)) {
	      resetppsnum=i;
	      errflag++;
	    }
	    else if (chipdiff!=1) {
	      chipsequenceerr=i;
	      errflag++;
	      

	    } // if the lab chip number is not sequential

	    if (trigNum_this<trigNum_previous && trigNum_previous!=pow(2,16)) {
	      resettrignum=i;
	      if (trigNum_previous!=65535) errflag++;
	    }

	   


// 	    // this was redundant
// 	    if (reserveddiff!=2) {
// 	      ferrs << "reserved[0] from event to event is not sequential.  Event = " << fRawEventPtr->eventNumber << " run= " << fCurrentRun << "(" << i << "th entry in the Header tree)\n";
// 	      ferrs << "reserved[0] for event " << fRawEventPtr->eventNumber << " is " << reserved_this << "\n";
// 	      ferrs << "reserved[0] for event " << eventNumber_previous << " is " << reserved_previous << "\n";
// 	    } // if the lab chip number is not sequential


	    //hdreservedlab->Fill(reserved_this-pow(2,labchip_this));

	    if (reserved_this-pow(2,labchip_this)!=0) {
	      errflag++;
	      chipreservederr=i;
	      
	    } // if reserved does not line up with labchip number
	    

	  } // if this is not the first event
	  
	} // only do this for the first channel because otherwise the information is redundant
      } // end loop over channels

 


    } // end loop over surfs

    for (int trigtype=0;trigtype<4;trigtype++) {
      

      //htriggerbits[trigtype]->Fill(fHeadPtr->trigType&(1<<trigtype));
      if (fHeadPtr->trigType&(1<<trigtype)){//<< moves the 1 over trigtype bits, then you and it with trigType
	mytriggerbits[trigtype]++;
	//htriggerTimeNs_pps1time->Fill(fHeadPtr->triggerTimeNs);
	//htriggerTimeNs[trigtype]->Fill(fHeadPtr->triggerTimeNs);
	
	//fill histograms for each priority
	//if (trigtype==0) htriggerTimeNsRF_priority[priority_this-1]->Fill(fHeadPtr->triggerTimeNs);
// 	if (trigtype==0){//for RF triggers, check if it is the ground pulser by time -- look at 650 ms
	  
// 	  if ((fHeadPtr->triggerTimeNs)<(groundPulserTime+groundPulserWindow)//count ground pulsers with given priority 
// 	      && (fHeadPtr->triggerTimeNs)>(groundPulserTime-groundPulserWindow)) {
// 	    n_ground_pulser[priority_this-1]++;  
// 	  }
// 	  //do an efficiency check for the double pulse
// 	  if (doublePulseFlag==1){//flag manually set if double pulse was going
// 	    if (last_event_was_ground_pulser==1){//was the last event a ground pulser?
// 	      if ((fHeadPtr->triggerTimeNs)<(groundPulserTime+groundPulserWindow) //then look for next event if it is in window
// 		  && (fHeadPtr->triggerTimeNs)>(groundPulserTime-groundPulserWindow)) {
// 		efficiency_numerator++;
// 		if (tTdiff<1000000){ 
// 		  average_doublePulse_triggerdiff+=tTdiff;
// 		  avg_ctr++;
// 		}
// 		//hgroundDoublePulse->Fill(tTdiff);
// 	      }
// 	      efficiency_denominator++;
// 	      last_event_was_ground_pulser=0;
// 	    }
// 	    else if ((fHeadPtr->triggerTimeNs)<(groundPulserTime+groundPulserWindow) 
// 		     && (fHeadPtr->triggerTimeNs)>(groundPulserTime-groundPulserWindow)){
// 	      last_event_was_ground_pulser=1;
// 	    }
// 	  }//end double pulse loop
// 	}//end rf triggers loop
	
      }//end trigtype loop
      if ((fHeadPtr->trigType&(1<<trigtype))!=0 && trigtype!=0) {
	//errflag++;
	triggerTypeFunny=i;
      }

    }

    //count number of events with sync slips
    if (surfidmismatch[0]!=-1 || surfidmismatch[1]!=-1 || surfidmismatch[2]!=-1 || 
	surfidmismatch[3]!=-1 || surfidmismatch[4]!=-1 || surfidmismatch[5]!=-1 || 
	surfidmismatch[6]!=-1 || surfidmismatch[7]!=-1 || surfidmismatch[8]!=-1 || 
	surfidmismatch[9]!=-1)sync_slips_surf_to_surf++;
    if ((surfturfidmismatch[0]!=-1 || surfturfidmismatch[1]!=-1 || surfturfidmismatch[2]!=-1 || 
	 surfturfidmismatch[3]!=-1 || surfturfidmismatch[4]!=-1 || surfturfidmismatch[5]!=-1 || 
	 surfturfidmismatch[6]!=-1 || surfturfidmismatch[7]!=-1 || surfturfidmismatch[8]!=-1 || 
	 surfturfidmismatch[9]!=-1) && (fRawEventPtr->surfEventId[0]!=0 || fRawEventPtr->surfEventId[1]!=0 ||
					fRawEventPtr->surfEventId[2]!=0 || fRawEventPtr->surfEventId[3]!=0 ||	
					fRawEventPtr->surfEventId[4]!=0 || fRawEventPtr->surfEventId[5]!=0 ||
					fRawEventPtr->surfEventId[6]!=0 || fRawEventPtr->surfEventId[7]!=0 ||
					fRawEventPtr->surfEventId[8]!=0 || fRawEventPtr->surfEventId[9]!=0 
					)) sync_slips_surf_to_turf++;
    if ((surfturfidmismatch[0]!=-1 || surfturfidmismatch[1]!=-1 || surfturfidmismatch[2]!=-1 || 
	 surfturfidmismatch[3]!=-1 || surfturfidmismatch[4]!=-1 || surfturfidmismatch[5]!=-1 || 
	 surfturfidmismatch[6]!=-1 || surfturfidmismatch[7]!=-1 || surfturfidmismatch[8]!=-1 || 
	 surfturfidmismatch[9]!=-1) && (fRawEventPtr->surfEventId[0]==0 || fRawEventPtr->surfEventId[1]==0 ||
					fRawEventPtr->surfEventId[2]==0 || fRawEventPtr->surfEventId[3]==0 ||	
					fRawEventPtr->surfEventId[4]==0 || fRawEventPtr->surfEventId[5]==0 ||
					fRawEventPtr->surfEventId[6]==0 || fRawEventPtr->surfEventId[7]==0 ||
					fRawEventPtr->surfEventId[8]==0 || fRawEventPtr->surfEventId[9]==0 
					)) sync_slips_surf_to_turf_0_surf_num++;

    


    if (errflag) {
      ferrs << "\n\n**************************************************\n";
      ferrs << "Run: " << fCurrentRun << "\t Event: " << fRawEventPtr->eventNumber << "\t Entry: " << i << "\n";
      ferrs << errflag << " errors.\n";
            
      for (int surf=0;surf<ACTIVE_SURFS;surf++) {
      if (surfidmismatch[surf]!=-1) {
	ferrs << "surfEventId's are not the same among surfs.\n";
	ferrs << "EventID for surf " << surf << " is " << fRawEventPtr->surfEventId[surf] << "\n";
	ferrs << "EventID for surf " << (surf+1)%ACTIVE_SURFS << " is " << fRawEventPtr->surfEventId[(surf+1)%ACTIVE_SURFS] << "\n\n";
      }
      if (surfturfidmismatch[surf]!=-1) {
	ferrs << "surfEventId's are not the same between surfs and the turf.\n";
	ferrs << "EventID for surf " << surf << " is " << fRawEventPtr->surfEventId[surf] << "\n";
	ferrs << "EventID for turf is " << fHeadPtr->turfEventId << "\n";
	ferrs << "High nybble of reserved[0] is " << ((fHeadPtr->reserved[0]&0xF0)>>4) << "\n\n";
      }
      
      for (int chan=0;chan<NUM_CHAN;chan++) {
	if (labchipmismatch[surf][chan]!=-1) {
	  ferrs << "Lab chips between neighboring channels not the same.\n";
	  int chanIndex=AnitaGeomTool::getChanIndex(surf,chan);
	  int chanIndex1=(chanIndex+1)%(ACTIVE_SURFS*NUM_CHAN);
	  int chanIndex2=chanIndex;
	  ferrs << "Lab chip for channel " << chanIndex1 << " is " << fRawEventPtr->getLabChip(chanIndex1) << "\n";
	  ferrs << "Lab chip for channel " << chanIndex2 << " is " << fRawEventPtr->getLabChip(chanIndex2) << "\n\n";
	  
	}

      } // loop over channels
    }

    if (chipsequenceerr!=-1 && resetppsnum==-1) {
      ferrs << "Chip number from event to event is not sequential, and there was no ppsNum reset.\n";
      ferrs << "Lab chip for event " << fRawEventPtr->eventNumber << " is " << labchip_this << "\n";
      ferrs << "Lab chip for event " << eventNumber_previous << " is " << labchip_previous << "\n";
      ferrs << "ppsNum for event " << fRawEventPtr->eventNumber << " is " << ppsNum_this << "\n";
      ferrs << "ppsNum for event " << eventNumber_previous << " is " << ppsNum_previous << "\n";
      ferrs << "Surf0 Event Num for event " << fRawEventPtr->eventNumber << " is " << surf0evnum_this << "\n";
      ferrs << "Surf1 Event Num for event " << eventNumber_previous << " is " << surf1evnum_previous << "\n";
      
    } // if the lab chip number is not sequential
    if (chipreservederr!=-1) {
      ferrs << "reserved[0] does not line up with labchip number.\n";
      ferrs << "reserved[0] is " << reserved_this << " and chip number is " << labchip_this << "\n";
      
    } // if reserved does not line up with labchip number

    if (resetppsnum!=-1) {
      ferrs << "PPSNUM RESET OCCURRED.\n";
    }
    if (resettrignum!=-1 && trigNum_previous!=65535) {
      ferrs << "TRIGNUM RESET OCCURRED.\n";
      ferrs << "trigNum for event " << fRawEventPtr->eventNumber << " is " << trigNum_this << "\n";
      ferrs << "trigNum for event " << eventNumber_previous << " is " << trigNum_previous << "\n";
    }
    if (sequential_event_numbers!=-1){
      ferrs << "Non-sequential event Numbers!\n";
      ferrs << "Event 1" << fRawEventPtr->eventNumber << "\n";
      ferrs << "Event 2" << eventNumber_previous << "\n";
    }


    ferrs << "**************************************************\n";
    ferrs << "\n\n\n";
    } // end if there are any errors this event


    eventNumber_previous=fRawEventPtr->eventNumber;
    labchip_previous=labchip_this;
    reserved_previous=reserved_this;
    tT_previous=tT_this;
    ppsNum_previous=ppsNum_this;
    trigNum_previous=trigNum_this;
    surf0evnum_previous=surf0evnum_this;
    surf1evnum_previous=surf1evnum_this;


  } // end loop over events

  cout << "Done looping over Event Tree.\n";

  cout << "About to write general errors.\n";
    ferrs << "**************************************************\n";
  ferrs << "Information about Run " << fCurrentRun << ":\n\n";
  
  if (mismatched_tree_sizes!=-1) {
    ferrs << "fSurfHkTree has more events than fEventTree.\n";
    ferrs << "Max event number in fSurfHkTree is " << fSurfHkTree->GetMaximum("eventNumber");
    ferrs << "Max event number in fEventTree is " << fEventTree->GetMaximum("eventNumber");
  }

  for (int ctr=0;ctr<fAcqTree->GetEntries();ctr++){
    fAcqTree->GetEvent(ctr);
    printf("%2.2X %2.2X\n", fAcqPtr->testBytes[6], fAcqPtr->testBytes[7]);
    ferrs << "Test bytes are: "<<fAcqPtr->testBytes[0]<<"\t"<<fAcqPtr->testBytes[1]<<"\t"
	  <<fAcqPtr->testBytes[2]<<"\t"
	  <<fAcqPtr->testBytes[3]<<"\t" 
	  <<dec<<((fAcqPtr->testBytes[5]&0xf0)>>4)<<"."<<(fAcqPtr->testBytes[5]&0xf)<<" build "
	  <<(fAcqPtr->testBytes[7]&0xf)<<"/"
	  <<(fAcqPtr->testBytes[6]&0xFF)<<" revision "
	  <<(fAcqPtr->testBytes[4]&0xFF)<<"\n";
  }
  if (!fSurfHkTree) 
    ferrs << "surfHkFile" << fCurrentRun << ".root does not exist.\n";
  
  /*  if (fSurfHkTree) {
    for (int surf=0;surf<ACTIVE_SURFS;surf++) {
      for (int scaler=0;scaler<SCALERS_PER_SURF;scaler++) {
	if (overflows[surf][scaler]!=-1) {
	  ferrs << "Surf " << surf << "\tScaler " << scaler << ":\t " << overflows[surf][scaler]*100. << " percent of the scalers are higher than " << scalerlimit[scaler] << "\n";
	}
      }  
    }
  
    
    
    ferrs << "Surf\tScaler\tMean \tRMS\n";
    for (int surf=0;surf<ACTIVE_SURFS;surf++) {
      for (int scaler=0;scaler<SCALERS_PER_SURF;scaler++) {
	
	
	
	
	hscalermean->Fill(log10(hscalers[surf][scaler]->GetMean()));
	hscalerrms->Fill(hscalers[surf][scaler]->GetRMS()/hscalers[surf][scaler]->GetMean());
	
	if (hscalers[surf][scaler]->GetRMS()==0) 
	  ferrs << surf << "\t" << scaler << "\t" << hscalers[surf][scaler]->GetMean() << "\t" << hscalers[surf][scaler]->GetRMS() << "\n";
	else if (rms_last10sec[surf][scaler]==0.)
	  ferrs << surf << "\t" << scaler << "\tRMS Scalers during last 10 seconds is " << rms_last10sec[surf][scaler] << "\n";
	
	
	
	
	
      }
    } // end loop over surfs
  } // end if surfhk exists
  */

  if (triggerTypeFunny!=-1) {
    //ferrs << "Some trigger types are not RF.\n";
    ferrs << "RF Triggers: " << mytriggerbits[0] << "\n";
    ferrs << "PPS1 Triggers: " << mytriggerbits[1] << "\n";
    ferrs << "PPS2 Triggers: " << mytriggerbits[2] << "\n";
    ferrs << "Soft Triggers: " << mytriggerbits[3] << "\n";
  }
  

  if(badC3poCount>0) {
    ferrs << "\nERROR:\t " << badC3poCount << " events have bad c3poNum\n";
  }

//   long int n_ground_pulser_total=0;
//   for (int ctr=0;ctr<9;ctr++){
//     n_ground_pulser_total+=n_ground_pulser[ctr];
//   }

//   //print out ground pulser priorities
//   ferrs<<"\n Check of Ground Pulser Event Priorities:\n"
//        <<"650 ms pulse: total number = "<<n_ground_pulser_total<<"\n";
//   for (int ctr=0;ctr<9;ctr++){
//     ferrs  <<"priority number "<<ctr+1<<": "<<n_ground_pulser[ctr]
// 	   <<"\t fraction of total: "
// 	   <<(double)n_ground_pulser[ctr]/(double)n_ground_pulser_total<<"\n";
//   }
//   ferrs <<"fraction with priority 1, 2, 3, or 4: "
// 	<<((double)n_ground_pulser[0]+(double)n_ground_pulser[1]
// 	   +(double)n_ground_pulser[2]+(double)n_ground_pulser[3])
//     /(double)n_ground_pulser_total<<"\n";
  
  ferrs<<"\n SUMMARY of run "<< fCurrentRun <<": "<<"\n"
       <<"Number of events with sync slips between surfs: "<<sync_slips_surf_to_surf<<"\n"
       <<"Number of events with sync slips between a surf and turf(surfid!=0): "<<sync_slips_surf_to_turf<<"\n"
       <<"Number of events with sync slips between a surf and turf(surfid==0): "<<sync_slips_surf_to_turf_0_surf_num
       <<"\n";
  
  //  efficiency_of_double_pulses=(double)efficiency_numerator/(double)efficiency_denominator;
  //  average_doublePulse_triggerdiff=average_doublePulse_triggerdiff/(double)avg_ctr;
  
  //print out trigger efficiency on second pulse in double pulse
 //  ferrs<<"\n Trigger Efficiency of Second Pulse of Ground Pulser Double Pulse: "<<efficiency_of_double_pulses<<"\n"
//        <<"Numerator (number of times second pulse triggered if first pulse triggered) is: "<<efficiency_numerator<<"\n"
//        <<"Denominator (number of times first pulse triggered) is: "<<efficiency_denominator<<"\n"
//        <<"Mean time between triggers when two pulsed: "<<average_doublePulse_triggerdiff<<"\n\n" ;
  

	   
  ferrs << "**************************************************\n";
  ferrs << "\n\n\n\n\n";
  //} //
  
  cout << "End writing general errors.\n";
  ferrs.close();
  
}

void lookForWaveformFunniness() //look for the same waveforms repeated event to event
                                               //also check dc offsets versus time between triggers
                                              //check trigger times to last 10 events if you find a repeated waveform
{

  loadEventTree();
  
  const int nfftpoints=256;//number of elements in fft arrays
  int numSurfs=10;//number of surfs
  int numEvents=fEventTree->GetEntries();
  cout <<numEvents<< "number of events"<<endl;
  int numChannels=1;//only check one channel per surf
  //int numChannels=9;//8 channels per surf plus clock
  int samectr[10]={0,0,0,0,0,0,0,0,0,0};
  double voltage_this[nfftpoints];//waveform voltage
  //double voltage_average;//waveform average
  double voltage_previous[10][numSurfs][numChannels][nfftpoints];//previous events voltage
  int trigTime_this=0;
  int trigTime_previous=0;
  int delta_trigTime[10]={0,0,0,0,0,0,0,0,0,0};
  //double average_previous[numSurfs][numChannels];
  int sameflag[10]={0,0,0,0,0,0,0,0,0,0}; //flag for checking that waveforms in consecutive events are the same
  //int issueBreakFlag=0;//for bailing from the points loop

  char filename[150];
  sprintf(filename,"%s/waveformFunniness_%d.txt",BASE_LOG_DIR,fCurrentRun);
  ofstream ferrors(filename);
  // sprintf(filename,"waveformFunninessTimes_%d.txt",fCurrentRun);
  //ofstream ferrorsTimes(filename);
  
  // TH2F *hdcOffset_vs_deltaTrigTime = new TH2F("dcOffset_vs_deltaTrigTime","Waveform Average (DC offset) versus time since previous trigger"
  //  ,100,0,1e9,100,-200,200.);
  //  TH2F *hdcOffset_vs_deltaTrigTime_next = new TH2F("dcOffset_vs_deltaTrigTime_next","Waveform Average (DC offset) versus time to next trigger"
  //				 ,100,0,1e9,100,-200,200.);
  // TH2F *hdcOffset_vs_deltaTrigTime_zoom = new TH2F("dcOffset_vs_deltaTrigTime_zoom","Waveform Average (DC offset) versus time since previous trigger"
  //					   ,1000000,0,1e8,100,-200,200.);
  //  TH2F *hdcOffset_vs_deltaTrigTime_next_zoom = new TH2F("dcOffset_vs_deltaTrigTime_next_zoom","Waveform Average (DC offset) versus time to next trigger"
  //						,1000000,0,1e8,100,-200,200.);
  
  // TH1F *hdeltaTrigTime_ifrepeatwaveform = new TH1F
  // ("hdeltaTrigTime_ifrepeatwaveform","time since previous trigger if it is the first instance of waveform repeating"
  // ,500,0,1e8);
 
  for (int eventctr=0;eventctr<numEvents;eventctr++){
    if (eventctr>=100){
      if (eventctr%(numEvents/10)==0) 
	cout<<eventctr<<" neutrinos.  "<<((double)eventctr/(double)numEvents)*100<< "% complete."<<endl;
    }
      
    fEventTree->GetEvent(eventctr);
    fHeadTree->GetEntryWithIndex(fRawEventPtr->eventNumber);
    fEventEntry=eventctr;
    int retVal=getEventEntry();
    if (eventctr==0) cout<<"First event number: "<<fRawEventPtr->eventNumber<<endl;
    if (eventctr==numEvents-1) cout<<"Last event number: "<<fRawEventPtr->eventNumber<<endl;

    trigTime_this=fHeadPtr->triggerTimeNs;//get this trigger time
    if (eventctr>10) delta_trigTime[0]=trigTime_this-trigTime_previous;//calculate trigger time increment
    else delta_trigTime[0]=0; //if eventctr<=10
    if (delta_trigTime[0]<0) delta_trigTime[0]+=1e9;
    
    for (int surfctr=0;surfctr<numSurfs;surfctr++){//surf loop
      for (int channelctr=0;channelctr<numChannels;channelctr++){
	//hdcOffset_vs_deltaTrigTime_next->Fill(delta_trigTime, average_previous[surfctr][channelctr]);
	//hdcOffset_vs_deltaTrigTime_next_zoom->Fill(delta_trigTime, average_previous[surfctr][channelctr]);
	//voltage_average=0;//reset average of waveform voltage.
	for (int ctr=0;ctr<nfftpoints;ctr++){//loop thru points in the waveform
	  //get this event voltage
	  voltage_this[ctr]=fUsefulEventPtr->fVolts[surfctr*9+channelctr][ctr];
	  //voltage_average+=voltage_this[ctr];
	  //check if any of the previous 10 events have the same point in the waveform and increment the flag.
	  for (int prev_number=0;prev_number<10;prev_number++){
	    if (voltage_previous[prev_number][surfctr][channelctr][ctr]==voltage_this[ctr]) 
	      sameflag[prev_number]++;
	    //else issueBreakFlag=1;
	  }
	  //if (issueBreakFlag==1) break;
	}
	//issueBreakFlag=0;
	for (int ctr=0;ctr<nfftpoints;ctr++){//loop thru points in the waveform
	  //move voltages to previous
	  for (int prev_number=9;prev_number>=0;prev_number--){
	    if (prev_number==0) voltage_previous[prev_number][surfctr][channelctr][ctr]=voltage_this[ctr];
	    if (prev_number!=0) voltage_previous[prev_number][surfctr][channelctr][ctr]=
	      voltage_previous[prev_number-1][surfctr][channelctr][ctr];
	  }
	}
	//voltage_average=voltage_average/(double)nfftpoints;
	//hdcOffset_vs_deltaTrigTime->Fill(delta_trigTime, voltage_average);
	//hdcOffset_vs_deltaTrigTime_zoom->Fill(delta_trigTime, voltage_average);
	//average_previous[surfctr][channelctr]=voltage_average;


	//if the sameness flag is as big as the number of points (-5 to be safe) then print this message.  Bad bad bad thing.
	for (int prev_number=0;prev_number<10;prev_number++){
	  if (sameflag[prev_number]>=nfftpoints-5 && samectr[prev_number]==0){ 
	    ferrors<<"\n"<<"Exact waveform repeated from this event number: "
		   <<fRawEventPtr->eventNumber<<" to this event number: "
		   <<(fRawEventPtr->eventNumber)-(prev_number+1)
		   <<"\n"
		   <<"First Surf Number: "<<surfctr+1
		   <<"\n";
	    ferrors <<"10 most recent deltaTrigTimes: ";
	    for (int prev_ctr=0;prev_ctr<10;prev_ctr++){
	      if (prev_ctr==9) ferrors<<delta_trigTime[prev_ctr]<<"\n\n";
	      else ferrors<<delta_trigTime[prev_ctr]<<", ";
	    }
	    //for (prev_number=0;prev_number<10;prev_number++){
	    // hdeltaTrigTime_ifrepeatwaveform->Fill(delta_trigTime[prev_number]);
	    //}
	    
	    samectr[prev_number]++;	    
	  }
	}
	
	for (int prev_number=0;prev_number<10;prev_number++){
	  sameflag[prev_number]=0;
	}
      }//end channel loop
    }//end surf loop
    for (int prev_number=0;prev_number<10;prev_number++){
      samectr[prev_number]=0;//reset to zero
    }
    trigTime_previous=trigTime_this;//set previous trigger time to this one
    for (int prev_number=9;prev_number>0;prev_number--){
      delta_trigTime[prev_number]=delta_trigTime[prev_number-1]; //get set previous = this one
    }
  }//end event loop

  ferrors.close();
  //TCanvas *c1=new TCanvas("c1","Time since previous trigger for waveform repeats",800,600);
  /*c1->Divide(2,1);
    TCanvas *c2=new TCanvas("c2","DC Offset vs. Time to next trigger",800,600);
    c2->Divide(2,1);
    c1->cd(1);
    hdcOffset_vs_deltaTrigTime->Draw("");
    c2->cd(1);
    hdcOffset_vs_deltaTrigTime_next->Draw("");
    c1->cd(2);
    hdcOffset_vs_deltaTrigTime_zoom->Draw("colz");
    c2->cd(2);
    hdcOffset_vs_deltaTrigTime_next_zoom->Draw("colz");
    
    sprintf(filename,"dcOffset_vs_deltaTrigTime_%d.eps",fCurrentRun);
    c1->Print(filename);
  */
  //  c1->cd(0);
  //hdeltaTrigTime_ifrepeatwaveform->Draw();

  //  sprintf(filename,"deltaTrigTime_ifwaveformrepeat_%d.eps",fCurrentRun);
  //c1->Print(filename);
 

}


void Zero(int *anarray,int n) {

  for (int i=0;i<n;i++) {
    anarray[i]=0;
  } //for

}
void MinusOnes(int *anarray,int n) {

  for (int i=0;i<n;i++) {
    anarray[i]=-1;
  } //for

}
void MinusOnes(double *anarray,int n) {

  for (int i=0;i<n;i++) {
    anarray[i]=-1;
  } //for

}
