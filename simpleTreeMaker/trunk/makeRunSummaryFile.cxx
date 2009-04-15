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
TFile *fHeadFile=0;
TTree *fHeadTree=0;
TChain *fEventTree=0;
RawAnitaHeader *fHeadPtr=0;
RawAnitaEvent *fRawEventPtr=0;


char fCurrentBaseDir[FILENAME_MAX];

void loadEventTree(int fCurrentRun);
void closeEventTree();

int main(int argc, char **argv) {
  if(argc<2) {
    std::cout << "Usage: " << basename(argv[0]) << " <base dir>" << std::endl;
    return -1;
  }
  strcpy(fCurrentBaseDir,argv[1]);

  ofstream LogFile("allRunSummary.txt");

  LogFile << "Run\tStartH\tEndH\tNumH\tStartE\tEndE\tNumE\tMatch\n";
  
 
  for(int run=1;run<263;run++) {
    //    if(run==24) continue;
    std::cout << run << "\n";
    loadEventTree(run);
    Long64_t numHeads=0;
    UInt_t firstHead=0;
    UInt_t lastHead=0;
    if(fHeadTree) {
      numHeads=fHeadTree->GetEntries();
      if(numHeads>0) {
	fHeadTree->GetEntry(0);
	firstHead=fHeadPtr->eventNumber;
	fHeadTree->GetEntry(numHeads-1);
	lastHead=fHeadPtr->eventNumber;
      }
    }
    Long64_t numEvents=0;
    UInt_t firstEvent=0;
    UInt_t lastEvent=0;
    if(fEventTree) {
      numEvents=fEventTree->GetEntries();
      if(numEvents>0) {
	fEventTree->GetEntry(0);
	firstEvent=fRawEventPtr->eventNumber;
	fEventTree->GetEntry(numEvents-1);
	lastEvent=fRawEventPtr->eventNumber;
      }
    }
    
    Int_t match=0;
    if(numHeads==numEvents && firstHead==firstEvent && lastHead==lastEvent)
      match=1;

    LogFile << run <<  "\t" << firstHead << "\t" << lastHead << "\t" << numHeads << "\t"
	    << firstEvent << "\t" << lastEvent << "\t" << numEvents << "\t"
	    << match << "\n";
    closeEventTree();
  }
  LogFile.close();

  return 0;
}




void loadEventTree(int fCurrentRun)
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
}

void closeEventTree() {
  if(fHeadFile) delete fHeadFile;
  if(fEventTree) delete fEventTree;
  fHeadFile=0;
  fHeadTree=0;
  fHeadPtr=0;
  fEventTree=0;
  fRawEventPtr=0; 
}
