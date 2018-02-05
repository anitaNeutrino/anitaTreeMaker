#include <cstdio>
#include <fstream>
#include <iostream>
#include <zlib.h>
#include <libgen.h>
#include <errno.h>
#include <vector> 


#include "TTree.h"
#include "TChain.h"
#include "TFile.h"
#include "TSystem.h"
#include "TH1.h"

#define HACK_FOR_ROOT

#include "RawAnitaHeader.h"
#include "RawAnitaEvent.h"
#include "CalibratedAnitaEvent.h"
#include "SurfHk.h"
#include "AcqdStart.h"
#include "AnitaGeomTool.h"
#include "UsefulAnitaEvent.h"
#include "AnitaConventions.h"
#include "AnitaEventCalibrator.h"

using namespace std;
//Some global variables
// TTree *fInputHeadTree=0;
// TFile *fInputHeadFile=0;
TChain *fInputEventChain=0;
TChain *fInputHeadChain=0;
RawAnitaHeader *fHeadPtr=0;
RawAnitaEvent *fRawEventPtr=0;
CalibratedAnitaEvent *fCalEventPtr=0;
AnitaEventCalibrator *fCal = AnitaEventCalibrator::Instance();

char fBaseDir[FILENAME_MAX];
char fOutBaseDir[FILENAME_MAX]; // Added fOutBaseDir because I don't have write permission at the moment
void loadEventTree(int fCurrentRun, Long64_t numPriorEventsToUse);
void closeEventTree();

int main(int argc, char **argv) {
  if(argc<4) {
    std::cerr << "Usage: " << basename(argv[0]) << " <base dir>  <out base dir> <run number>" << std::endl;
    return -1;
  }
  strcpy(fBaseDir,argv[1]);
  strcpy(fOutBaseDir,argv[2]);
  Int_t run=atoi(argv[3]);
  std::vector<Double_t> rcoArray(NUM_SURF, 1);
  std::vector<Double_t> clockPhiArray(NUM_SURF, 1);
  std::vector<Double_t> tempFactorGuesses(NUM_SURF, 1);
  Int_t clockProblem;
  Int_t clockSpike;
  Int_t rfSpike;
  
  char outEventName[FILENAME_MAX];
  sprintf(outEventName,"%s/run%d/calibratedEventInfo%d.root",fOutBaseDir,run,run);

  // const Int_t numPriorEvents = 500000; 
  const Int_t numPriorEvents = 5000;
  loadEventTree(run, numPriorEvents);

  if(fInputEventChain->GetEntries()==0 || fInputEventChain->GetEntries()==0) {
    std::cout << "No data for run " << run << "\n";
    return -1;
  }
  else{
    std::cout << fInputEventChain->GetEntries() << " events to process." << std::endl;
  }
  
  TFile *outEventFile = new TFile(outEventName,"RECREATE");
  TTree* outEventTree = new TTree("calInfoTree","Tree of Calibration Info");
  outEventTree->Branch("run",&run,"run/I");
  outEventTree->Branch("rcoArray",&rcoArray);
  outEventTree->Branch("clockPhiArray",&clockPhiArray);
  outEventTree->Branch("tempFactorGuesses",&tempFactorGuesses);
  outEventTree->Branch("clockProblem",&clockProblem);
  outEventTree->Branch("clockSpike",&clockSpike);
  outEventTree->Branch("rfSpike",&rfSpike);

  Long64_t numEvents=fInputEventChain->GetEntries();

  Int_t starEvery=numEvents/1000;
  if(starEvery==0) starEvery=1;

  std::cout << "Starting run: " << run << "\n";
  std::cerr << "Starting run: " << run << "\n";

  for(Long64_t eventEntry=0; eventEntry<numEvents; eventEntry++) {
    if(eventEntry%starEvery == 0) 
      std::cerr << "*";

    fInputHeadChain->GetEntry(eventEntry);

    // debugging
    // if(fHeadPtr->eventNumber < 31455791){
    //   continue;
    // }

    // // debugging
    // if(fHeadPtr->eventNumber < 10512380 ) continue;

    fInputEventChain->GetEntry(eventEntry);
    // UsefulAnitaEvent realEvent(fRawEventPtr,WaveCalType::kDefault,fHeadPtr);
    // UsefulAnitaEvent realEvent(fRawEventPtr,WaveCalType::kNoTriggerJitterNoZeroMean,fHeadPtr);

    UsefulAnitaEvent realEvent(fRawEventPtr,WaveCalType::kDefault,fHeadPtr);    
    if(fCalEventPtr){
      delete fCalEventPtr;
      fCalEventPtr = NULL;
    }      
    
    fCalEventPtr = new CalibratedAnitaEvent(&realEvent);

    clockProblem = fCalEventPtr->fClockProblem;
    clockSpike = fCalEventPtr->fClockSpike;
    rfSpike = fCalEventPtr->fRFSpike;
    for(int surf = 0; surf < NUM_SURF; surf++)
    {
      rcoArray[surf] = fCalEventPtr->fRcoArray[surf];
      clockPhiArray[surf] = fCalEventPtr->fClockPhiArray[surf];
      tempFactorGuesses[surf] = fCalEventPtr->fTempFactorGuesses[surf];
    }
    outEventTree->Fill();
  }
  outEventTree->AutoSave();
  outEventFile->Close();
  std::cerr << "\n";
  return 0;
}




void loadEventTree(int fCurrentRun, Long64_t numPriorEventsToUse)
{       

  // This function has been modified to get some number of events into the RingBuffer 
  // in AnitaEventCalibrator.
  // For previous behaviour, pass numPriorEventsToUse=0.

  char eventName[FILENAME_MAX];
  char headerName[FILENAME_MAX];

  int priorRun = fCurrentRun;
  Long64_t numPriorEvents = 0;

  fInputEventChain = NULL;
  fInputHeadChain = NULL;

  std::cout << "numPriorEventsToUse = " << numPriorEventsToUse << std::endl;

  // Here we open prior runs (if they exist) to loop over and work out how many we need,
  // the answer is probably just one.
  while(priorRun >= 1 && numPriorEvents < numPriorEventsToUse){
    priorRun--;
    sprintf(eventName,"%s/run%d/eventFile%d.root",fBaseDir,priorRun,priorRun);
    TFile* fPriorEventFile = TFile::Open(eventName);
    if(fPriorEventFile!=NULL){
      TTree* fPriorTree = (TTree*) fPriorEventFile->Get("eventTree");
      numPriorEvents += fPriorTree->GetEntries();
      std::cout << "priorRun = " << priorRun << "\t";
      std::cout << "numPriorEvents = " << numPriorEvents << std::endl;
      fPriorEventFile->Close();
    }
  }

  if(numPriorEvents > 0){

    // Create raw event and header chains
    fInputEventChain = new TChain("eventTree");
    fInputHeadChain = new TChain("headTree");

    // Add prior runs to those chains
    for(int run=priorRun; run<fCurrentRun; run++){
      sprintf(eventName,"%s/run%d/eventFile%d.root",fBaseDir,run,run);
      fInputEventChain->Add(eventName);
      fInputEventChain->SetBranchAddress("event",&fRawEventPtr);

      sprintf(headerName,"%s/run%d/headFile%d.root",fBaseDir,run,run);
      fInputHeadChain->Add(headerName);
      fInputHeadChain->SetBranchAddress("header",&fHeadPtr);
    }

    // Get number of events to skip before adding fCurrentRun
    Long64_t probWayTooManyEventsPrior = fInputEventChain->GetEntries();
    Long64_t numToSkip = probWayTooManyEventsPrior - numPriorEventsToUse;

    std::cout << "probWayTooManyEventsPrior = " << probWayTooManyEventsPrior << std::endl;
    std::cout << "numToSkip = " << numToSkip << std::endl;

    // Create the AnitaEventCalibrator in this loop, filling up RingBuffer of clock periods for temp correction
    for(Long64_t entry=numToSkip; entry<probWayTooManyEventsPrior; entry++){
      fInputHeadChain->GetEntry(entry);
      fInputEventChain->GetEntry(entry);

      // Don't do time consuming trigger jitter correction, we only care about temperature correction
      UsefulAnitaEvent usefulEventToDoTempCorrection(fRawEventPtr, 
						     WaveCalType::kNoTriggerJitterNoZeroMean, 
						     fHeadPtr);
    }
  }

  std::cout << "Did you create an instance of AnitaEventCalibrator?" << std::endl;
  std::cout << "If you did, make sure you don't create another!" << std::endl;
  std::cout << "If you didn't that's OK for run 1." << std::endl;

  // delete the chain we just looped over to fill the RingBuffer in AnitaEventCalibrator
  // (if we did loop over anything)
  if(fInputEventChain!=NULL) delete fInputEventChain;
  fInputEventChain = new TChain("eventTree");
  if(fInputHeadChain!=NULL) delete fInputHeadChain;
  fInputHeadChain = new TChain("headTree");

  // Add fCurrentRun to our new chain...
  sprintf(eventName,"%s/run%d/eventFile%d*.root",fBaseDir,fCurrentRun,fCurrentRun);
  fInputEventChain->Add(eventName);
  fInputEventChain->SetBranchAddress("event",&fRawEventPtr);
  sprintf(headerName,"%s/run%d/headFile%d.root",fBaseDir,fCurrentRun,fCurrentRun);
  fInputHeadChain->Add(headerName);
  fInputHeadChain->SetBranchAddress("header",&fHeadPtr);
  
}

void closeEventTree() {
  if(fInputHeadChain) delete fInputHeadChain;
  if(fInputEventChain) delete fInputEventChain;
  fInputEventChain=0;
  fInputHeadChain=0;
  fHeadPtr=0;
  fInputEventChain=0;
  fRawEventPtr=0; 
}
