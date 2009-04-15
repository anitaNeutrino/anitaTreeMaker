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
#include "CalibratedAnitaEvent.h"
#include "SurfHk.h"
#include "AcqdStart.h"
#include "AnitaGeomTool.h"
#include "UsefulAnitaEvent.h"

using namespace std;
//Some global variables
TTree *fHeadTree=0;
TFile *fHeadFile=0;
TChain *fEventChain=0;
RawAnitaHeader *fHeadPtr=0;
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
  
  loadEventTree(run);
  if(fEventChain->GetEntries()==0 || !fHeadTree) {
    std::cout << "No data for run " << run << "\n";
    return -1;
  }

  Long64_t numEvents=fEventChain->GetEntries();


  Int_t starEvery=numEvents/1000;
  if(starEvery==0) starEvery=1;

  std::cout << "Starting run: " << run << "\n";
  std::cerr << "Starting run: " << run << "\n";


  char outName[FILENAME_MAX];
  sprintf(outName,"zeroHists%d.root",run);
  TFile *fpOut = new TFile(outName,"RECREATE");
  TH1F *histZero = new TH1F("histZero","histZero",2700,-0.5,2699.5);
  TTree *zeroTree = new TTree("zeroTree","zeroTree");
  Int_t surfNum;
  Int_t numZeros;
  UInt_t eventNumber;
  zeroTree->Branch("surf",&surfNum,"surf/I");
  zeroTree->Branch("numZeros",&numZeros,"numZeros/I");
  zeroTree->Branch("eventNumber",&eventNumber,"eventNumber/i");

  char corruptEventList[FILENAME_MAX];
  sprintf(corruptEventList,"/home/rjn/anita/errorLogs/corrupt/corruptEvent%d.txt",run);
  ofstream EventList(corruptEventList);



  Int_t countBad=0;
  for(Long64_t eventEntry=0;eventEntry<numEvents;eventEntry++) {
    if(eventEntry%starEvery == 0) 
      std::cerr << "*";
      fHeadTree->GetEntry(eventEntry);
      fEventChain->GetEntry(eventEntry);
      

      Int_t eventBad=0;
      Int_t surfBad[NUM_SURF]={0};
      for(int surf=0;surf<NUM_SURF;surf++) {
	Int_t countZeros=0;
	for(int chan=0;chan<NUM_CHAN;chan++) {
	  int chanIndex=AnitaGeomTool::getChanIndex(surf,chan);
	  for(int samp=0;samp<NUM_SAMP;samp++) {
	    if(fCalEventPtr->data[chanIndex][samp]==0) {
	      countZeros++;
	    }
	  }	  
	}
	histZero->Fill(countZeros);
	numZeros=countZeros;
	surfNum=surf;
	eventNumber=fCalEventPtr->eventNumber;
	zeroTree->Fill();
	
	if(numZeros>400) {
	  eventBad=1;
	  surfBad[surf]=1;
	}	
      }
      if(eventBad) {
	countBad++;
	EventList << eventNumber << "\t";
	for(int surf=0;surf<NUM_SURF;surf++) {
	  if(surfBad[surf])
	    EventList << surf+1 << "\t";
	}
	EventList << "\n";
      }       
  }  
  
  zeroTree->AutoSave();
  fpOut->Write();
  fpOut->Close();

  EventList.close();

  char corruptSummary[FILENAME_MAX];
  sprintf(corruptSummary,"/home/rjn/anita/errorLogs/corrupt/corruptSum%d.txt",run);
  ofstream Summary(corruptSummary);
  
  Summary << "**************\n";
  Summary << "Run " << run << " has " << countBad << " corrupted events\n";
  Summary << "**************\n";
  Summary.close();
  
  std::cerr << "\n"; 
  return 0;
}




void loadEventTree(int fCurrentRun)
{       
  char eventName[FILENAME_MAX];
  char headerName[FILENAME_MAX];
  sprintf(eventName,"%s/run%d/calEventFile%d*.root",fBaseDir,fCurrentRun,fCurrentRun);
  sprintf(headerName,"%s/run%d/headFile%d.root",fBaseDir,fCurrentRun,fCurrentRun);
  fEventChain = new TChain("eventTree");
  fEventChain->Add(eventName);
  fEventChain->SetBranchAddress("event",&fCalEventPtr);

      
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
}

void closeEventTree() {
  if(fHeadFile) delete fHeadFile;
  if(fEventChain) delete fEventChain;
  fHeadFile=0;
  fHeadTree=0;
  fHeadPtr=0;
  fEventChain=0;
  fCalEventPtr=0; 
}
