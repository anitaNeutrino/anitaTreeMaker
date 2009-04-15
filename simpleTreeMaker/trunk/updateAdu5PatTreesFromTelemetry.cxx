#include <cstdio>
#include <fstream>
#include <iostream>
#include <zlib.h>
#include <libgen.h>
#include <errno.h>
   


#include "TTree.h"
#include "TTreeIndex.h"
#include "TChain.h"
#include "TFile.h"
#include "TSystem.h"
#include "TH1.h"

#define HACK_FOR_ROOT

#include "RawAnitaHeader.h"
#include "Adu5Pat.h"
#include "AnitaGeomTool.h"
#include "UsefulAnitaEvent.h"

using namespace std;
//Some global variables
TFile *fHeadFile=0;
TTree *fHeadTree=0;
RawAnitaHeader *fHeadPtr=0;
Adu5Pat *fThePatPtr=0;





char fBaseDir[FILENAME_MAX];


void loadHeaderFiles(int fCurrentRun);
void closeHeaderTree();

int main(int argc, char **argv) {
  if(argc<2) {
    std::cout << "Usage: " << basename(argv[0]) << " <base dir>" << std::endl;
    return -1;
  }
  strcpy(fBaseDir,argv[1]);

  TFile *fpTelem = new TFile("/unix/anita/webPlotterDataTesting/gpsFile.root");
  TTree *adu5TelemTree = (TTree*) fpTelem->Get("adu5PatTree");
  if(!adu5TelemTree) {
    std::cerr << "Couldn't get adu5PatTree telemetry tree\n";
    return -1;
  }
  adu5TelemTree->BuildIndex("realTime");
  TTreeIndex *telemTreeIndex = (TTreeIndex*) adu5TelemTree->GetTreeIndex();
  adu5TelemTree->SetBranchAddress("pat",&fThePatPtr);

  Int_t badRunArray[13]={203,209,213,222,225,228,229,231,235,237,245,247,250};
  Long64_t runStartTimes[13]={0};
  Long64_t runEndTimes[13]={0};

  
  for(Int_t runInd=0;runInd<13;runInd++) {
    Int_t run=badRunArray[runInd];
    loadHeaderFiles(run);
    if(!fHeadTree) {
      std::cout << "No event data for run " << run << "\n";
      continue;
    }
    if(fHeadTree->GetEntries()!=0) {
      fHeadTree->GetEntry(0);
      runStartTimes[runInd]=fHeadPtr->realTime;
    }
    closeHeaderTree();   
    run++;
    loadHeaderFiles(run);
    if(!fHeadTree) {
      std::cout << "No event data for run " << run << "\n";
      continue;
    }
    if(fHeadTree->GetEntries()!=0) {
      fHeadTree->GetEntry(0);
      runEndTimes[runInd]=fHeadPtr->realTime;
    }

    

    std::cout << run << "\t" << runStartTimes[runInd] << "\t" << runEndTimes[runInd] << "\n";
  }


  Long64_t *telemIndex= telemTreeIndex->GetIndex();
  Long64_t *telemIndexValues = telemTreeIndex->GetIndexValues();
  Long64_t numTelem = telemTreeIndex->GetN();
  Int_t currentRunInd=0;
  TFile *fpCurrentGpsFile=0;
  TTree *fCurrentAdu5PatTree=0;

  char gpsName[180];

  for(int i=0;i<numTelem;i++) {
    Long64_t realTime= (telemIndexValues[i]>>31);

    if(realTime>runEndTimes[currentRunInd]) {
      if(fCurrentAdu5PatTree) {
	fCurrentAdu5PatTree->AutoSave();
	fpCurrentGpsFile->Close();
	fCurrentAdu5PatTree=0;
	fpCurrentGpsFile=0;
      }
      currentRunInd++;
      if(currentRunInd>=13) break;
    }
    if(realTime>runStartTimes[currentRunInd]) {
      fThePatPtr=0;
      if(!fpCurrentGpsFile) {
	sprintf(gpsName,"%s/run%d/gpsFile%d.root",fBaseDir,badRunArray[currentRunInd],badRunArray[currentRunInd]);
	fpCurrentGpsFile = new TFile(gpsName,"UPDATE");
	fCurrentAdu5PatTree = (TTree*) fpCurrentGpsFile->Get("adu5PatTree");
	fCurrentAdu5PatTree->SetBranchAddress("pat",&fThePatPtr);
      }
      std::cout << telemIndex[i] << "\t" << realTime << "\t" << badRunArray[currentRunInd] << "\n";
      adu5TelemTree->GetEntry(telemIndex[i]);
      std::cout << fThePatPtr->heading << "\n";
      fCurrentAdu5PatTree->Fill();
    }
  }



  

  return 0;
}




void loadHeaderFiles(int fCurrentRun)
{       
  char headerName[FILENAME_MAX];
  sprintf(headerName,"%s/run%d/headFile%d.root",fBaseDir,fCurrentRun,fCurrentRun);
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

void closeHeaderTree() {
  if(fHeadFile) delete fHeadFile;
  fHeadFile=0;
  fHeadTree=0;
  fHeadPtr=0;

}
