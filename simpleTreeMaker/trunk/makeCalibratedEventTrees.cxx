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
#include "CalibratedAnitaEvent.h"
#include "SurfHk.h"
#include "AcqdStart.h"
#include "AnitaGeomTool.h"
#include "UsefulAnitaEvent.h"

using namespace std;
//Some global variables
TTree *fInputHeadTree=0;
TFile *fInputHeadFile=0;
TChain *fInputEventChain=0;
RawAnitaHeader *fHeadPtr=0;
RawAnitaEvent *fRawEventPtr=0;
CalibratedAnitaEvent *fCalEventPtr=0;


char fBaseDir[FILENAME_MAX];
void loadEventTree(int fCurrentRun);
void closeEventTree();

int main(int argc, char **argv) {
  if(argc<3) {
    std::cout << "Usage: " << basename(argv[0]) << " <base dir>  <run number>" << std::endl;
    return -1;
  }
  strcpy(fBaseDir,argv[1]);
  Int_t run=atoi(argv[2]);
  
  char outEventName[FILENAME_MAX];
  sprintf(outEventName,"%s/run%d/calEventFile%d.root",fBaseDir,run,run);

 
  loadEventTree(run);
  if(fInputEventChain->GetEntries()==0 || !fInputHeadTree) {
    std::cout << "No data for run " << run << "\n";
    return -1;
  }

  
  TFile *outEventFile = new TFile(outEventName,"RECREATE");
  TTree* outEventTree = new TTree("eventTree","Tree of Anita Events");
  outEventTree->Branch("run",&run,"run/I");
  outEventTree->Branch("event","CalibratedAnitaEvent",&fCalEventPtr);


  Long64_t numEvents=fInputEventChain->GetEntries();


  Int_t starEvery=numEvents/1000;
  if(starEvery==0) starEvery=1;

  std::cout << "Starting run: " << run << "\n";
  std::cerr << "Starting run: " << run << "\n";

  for(Long64_t eventEntry=0;eventEntry<numEvents;eventEntry++) {
    if(eventEntry%starEvery == 0) 
      std::cerr << "*";
      fInputHeadTree->GetEntry(eventEntry);
      fInputEventChain->GetEntry(eventEntry);
      UsefulAnitaEvent realEvent(fRawEventPtr,WaveCalType::kDefault,fHeadPtr);
      if(fCalEventPtr)
	delete fCalEventPtr;
      fCalEventPtr = new CalibratedAnitaEvent(&realEvent);
      outEventTree->Fill();

  }
  outEventTree->AutoSave();
  std::cerr << "\n";
  return 0;
}




void loadEventTree(int fCurrentRun)
{       
  char eventName[FILENAME_MAX];
  char headerName[FILENAME_MAX];
  sprintf(eventName,"%s/run%d/eventFile%d*.root",fBaseDir,fCurrentRun,fCurrentRun);
  sprintf(headerName,"%s/run%d/headFile%d.root",fBaseDir,fCurrentRun,fCurrentRun);
  fInputEventChain = new TChain("eventTree");
  fInputEventChain->Add(eventName);
  fInputEventChain->SetBranchAddress("event",&fRawEventPtr);

      
  fInputHeadFile = new TFile(headerName);
  if(!fInputHeadFile) {
    cout << "Couldn't open: " << headerName << "\n";
    return;
  }
  fInputHeadTree = (TTree*) fInputHeadFile->Get("headTree");
  if(!fInputHeadTree) {
    cout << "Couldn't get headTree from " << headerName << endl;
    return;
  }
  fInputHeadTree->SetBranchAddress("header",&fHeadPtr);
}

void closeEventTree() {
  if(fInputHeadFile) delete fInputHeadFile;
  if(fInputEventChain) delete fInputEventChain;
  fInputHeadFile=0;
  fInputHeadTree=0;
  fHeadPtr=0;
  fInputEventChain=0;
  fRawEventPtr=0; 
}
