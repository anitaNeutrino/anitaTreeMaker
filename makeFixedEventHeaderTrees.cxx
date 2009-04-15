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

using namespace std;
//Some global variables
TFile *fInputHeadFile=0;
TTree *fInputHeadTree=0;
TChain *fInputEventChain=0;
RawAnitaHeader *fHeadPtr=0;
RawAnitaEvent *fRawEventPtr=0;


char fInputBaseDir[FILENAME_MAX];
char fOutputBaseDir[FILENAME_MAX];

void loadInputTrees(int fCurrentRun);
void closeEventTree();

int main(int argc, char **argv) {
  if(argc<4) {
    std::cout << "Usage: " << basename(argv[0]) << " <input dir> <output dir> <run number>" << std::endl;
    return -1;
  }
  strcpy(fInputBaseDir,argv[1]);
  strcpy(fOutputBaseDir,argv[2]);
  Int_t run=atoi(argv[3]);
  
  char outHeadName[FILENAME_MAX];
  sprintf(outHeadName,"%s/run%d/headFile%d.root",fOutputBaseDir,run,run);
  char outEventName[FILENAME_MAX];
  sprintf(outEventName,"%s/run%d/eventFile%d.root",fOutputBaseDir,run,run);

 
  loadInputTrees(run);
  if(!fInputHeadTree || fInputEventChain->GetEntries()==0) {
    std::cout << "No data for run " << run << "\n";
    return -1;
  }


  TFile *outHeadFile = new TFile(outHeadName,"RECREATE");  
  TTree *outHeadTree = new TTree("headTree","Tree of Anita Event Headers");
  outHeadTree->Branch("header","RawAnitaHeader",&fHeadPtr);

  
  TFile *outEventFile = new TFile(outEventName,"RECREATE");
  TTree* outEventTree = new TTree("eventTree","Tree of Anita Events");
  outEventTree->Branch("run",&run,"run/I");
  outEventTree->Branch("event","RawAnitaEvent",&fRawEventPtr);


  Long64_t numHeads=fInputHeadTree->GetEntries();
  Long64_t numEvents=fInputEventChain->GetEntries();

  Long64_t headEntry=0;
  Long64_t eventEntry=0;

  std::cout << numHeads << "\t" << numEvents << "\n";

  Long64_t loopCounter=0;
  while (1) {
    loopCounter++;
    if(loopCounter%1000 ==0 ) {
      std::cout << headEntry << "\t" << eventEntry << "\n";
    }
    if(headEntry<numHeads) {
      fInputHeadTree->GetEntry(headEntry);
    }
    else {
      break;
    }
    if(eventEntry<numEvents) {
      fInputEventChain->GetEntry(eventEntry);
    }
    else {
      break;
    }
    
    if(fRawEventPtr->eventNumber == fHeadPtr->eventNumber) {
      //Wahey we have a match
      outHeadTree->Fill();
      outEventTree->Fill();
      headEntry++;
      eventEntry++;
    }
    else {
      //Not a match
      if(fRawEventPtr->eventNumber<fHeadPtr->eventNumber) {
	eventEntry++;
	continue;
      }
      else {
	headEntry++;
	continue;
      }
    }    
  }



  outEventTree->AutoSave();
  outHeadTree->AutoSave();
  

  return 0;
}




void loadInputTrees(int fCurrentRun)
{       
  char eventName[FILENAME_MAX];
  char headerName[FILENAME_MAX];
  sprintf(eventName,"%s/run%d/eventFile%d*.root",fInputBaseDir,fCurrentRun,fCurrentRun);
  sprintf(headerName,"%s/run%d/headFile%d.root",fInputBaseDir,fCurrentRun,fCurrentRun);
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
