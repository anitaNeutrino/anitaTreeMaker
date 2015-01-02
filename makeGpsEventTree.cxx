#include <cstdio>
#include <fstream>
#include <iostream>
#include <zlib.h>
#include <libgen.h>      

#include "TTree.h"
#include "TFile.h"
#include "TSystem.h"
#include "TTreeIndex.h"
#include "TMath.h"

#include "Adu5Pat.h"
#include "RawAnitaHeader.h"

#include <map>

using namespace std;

void makeGpsEventTree(char *inName, char *headName, char *outName);


int main(int argc, char **argv) {
  if(argc<3) {
    std::cout << "Usage: " << basename(argv[0]) << " <root dir> <run num> <outhkfile>" << std::endl;
    return -1;
  }
  makeGpsEventTree(argv[1],atoi(argv[2]),argv[3]);
  return 0;
}


void makeGpsEventTree(char *rootDir, int runNum, char *outName) {

  Adu5Pat *patPtr=0;
  RawAnitaHeader *headPtr=0;
  char inName[FILENAME_MAX];
  char headName[FILENAME_MAX];
  sprintf(headName,"%s/run%d/headFile%d.root",rootDir,runNum,runNum);

  TChain *adu5PatTree = new TChain("adu5PatTree");
  for(int testRun=runNum-1;testRun<runNum+2;runNum++) {
    sprintf(inName,"%s/run%d/gpsFile%d.root",rootDir,runNum,runNum);
    adu5PatTree->AddFile(inName);
  }
   adu5PatTree->SetBranchAddress("pat",&patPtr);   
   TFile *fpHead = new TFile(headName);
   if(!fpHead) {
      std::cerr << "Couldn't open " << headName << "\n";
      return;
   }  
   TTree *headTree = (TTree*) fpHead->Get("headTree");
   if(!headTree) {
      std::cerr << "Couldn't get headTree from " << headName << "\n";
      return;
   }      
   headTree->SetBranchAddress("header",&headPtr);
      

   TFile *fpOut = new TFile(outName,"RECREATE");
   TTree *adu5PatTreeInt = new TTree("adu5PatTree","Tree of ADU5 PAT");
   Adu5Pat *thePat = new Adu5Pat();
   adu5PatTreeInt->Branch("pat","Adu5Pat",&thePat);

   Long64_t headEntries = headTree->GetEntries();
   Long64_t nentries = adu5PatTree->GetEntries();
   adu5PatTree->BuildIndex("realTime","payloadTimeUs");

   Long64_t nbytes = 0, nb = 0;
   Int_t intFlag;

   std::map<Long64_t,Int_t> adu5PatEntryMap;
   for(int entry=0;entry<nentries;entry++) {
     adu5PatTree->GetEntry(entry);
     if(patPtr->attFlag==0) {
       Long64_t fakeTime=patPtr->realTime;
       adu5PatEntryMap.insert(std::pair<Long64_t,Int_t>(fakeTime , entry)  );
     }
   }
   
   std::cout << "adu5PatEntryMap.size(): "<< adu5PatEntryMap.size() << "\n";

   std::map<Long64_t,Int_t>::iterator patLowIt; 
   std::map<Long64_t,Int_t>::iterator patUpIt; 

   for (Long64_t jentry=0; jentry<headEntries;jentry++) {
      if(jentry%10000==0) cerr << "*";
      nb = headTree->GetEntry(jentry);   nbytes += nb;
     

      Long64_t triggerTime=headPtr->triggerTime;
      patUpIt=adu5PatEntryMap.upper_bound(triggerTime);
      Int_t upIndex=patUpIt->second;
      Int_t downIndex=0;
      if(upIndex>0) downIndex=upIndex-1;
 
      std::cout << triggerTime << "\t" << downIndex << "\t" << upIndex << "\n";
      adu5PatTree->GetEntry(upIndex);


      intFlag=headPtr->triggerTime-patPtr->realTime;

      if(thePat) delete thePat;
      thePat = new Adu5Pat(*patPtr);
      thePat->intFlag=intFlag;
      adu5PatTreeInt->Fill();

   }
   adu5PatTreeInt->AutoSave();
   fpOut->Close();
   cerr << endl;
}
