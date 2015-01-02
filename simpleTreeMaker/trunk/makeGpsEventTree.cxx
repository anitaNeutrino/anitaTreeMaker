#include <cstdio>
#include <fstream>
#include <iostream>
#include <zlib.h>
#include <libgen.h>      

#include "TTree.h"
#include "TChain.h"
#include "TFile.h"
#include "TSystem.h"
#include "TTreeIndex.h"
#include "TMath.h"

#include "Adu5Pat.h"
#include "RawAnitaHeader.h"

#include <map>

using namespace std;

void makeGpsEventTree(char *rootDir, int runNum);

int main(int argc, char **argv) {
  if(argc<2) {
    std::cout << "Usage: " << basename(argv[0]) << " <root dir> <run num> " << std::endl;
    return -1;
  }
  makeGpsEventTree(argv[1],atoi(argv[2]));
  return 0;
}


void makeGpsEventTree(char *rootDir, int runNum) {

  Adu5Pat *patPtr=0;
  RawAnitaHeader *headPtr=0;
  char inName[FILENAME_MAX];
  char outName[FILENAME_MAX];
  char headName[FILENAME_MAX];
  sprintf(headName,"%s/run%d/headFile%d.root",rootDir,runNum,runNum);
  sprintf(outName,"%s/run%d/gpsEvent%d.root",rootDir,runNum,runNum);

  TChain *adu5PatTree = new TChain("adu5PatTree");
  for(int testRun=runNum-1;testRun<runNum+2;testRun++) {
    sprintf(inName,"%s/run%d/gpsFile%d.root",rootDir,testRun,testRun);
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
      patLowIt=patUpIt;
      if(patLowIt!=adu5PatEntryMap.begin()) {
	patLowIt--;
	downIndex=patLowIt->second;
      }
 
      //      std::cout << triggerTime << "\t" << downIndex << "\t" << upIndex << "\n";
      adu5PatTree->GetEntry(downIndex);
      Double_t lowTime=patPtr->realTime;
      Double_t lowLat=patPtr->latitude;
      Double_t lowLong=patPtr->longitude;
      Double_t lowAlt=patPtr->altitude;
      Double_t lowHeading=patPtr->heading;

      //      std:: cout << "\t" << patPtr->realTime << "\t";
      adu5PatTree->GetEntry(upIndex);

      //      std:: cout << patPtr->realTime << "\n";
      Double_t upTime=patPtr->realTime;
      Double_t upLat=patPtr->latitude;
      Double_t upLong=patPtr->longitude;
      Double_t upAlt=patPtr->altitude;
      Double_t upHeading=patPtr->heading;

      
      Double_t diffTime=TMath::Abs(upTime-headPtr->triggerTime);
      Double_t diffTime2=TMath::Abs(lowTime-headPtr->triggerTime);
      if(diffTime2<diffTime) diffTime=diffTime2;

      Double_t timeRat=(headPtr->triggerTime-lowTime)/(upTime-lowTime);
      Double_t bestLat=lowLat+timeRat*(upLat-lowLat);
      if((upLong-lowLong)>180) upLong-=360;
      if((upLong-lowLong)<-180) upLong+=360;
      Double_t bestLong=lowLong+timeRat*(upLong-lowLong); 
      Double_t bestAlt=lowAlt+timeRat*(upAlt-lowAlt); 
      if((upHeading-lowHeading)>180) upHeading-=360;
      if((upHeading-lowHeading)<-180) upHeading+=360;
      Double_t bestHeading=lowHeading+timeRat*(upHeading-lowHeading); 
      patPtr->realTime=headPtr->triggerTime;
      patPtr->latitude=bestLat;
      patPtr->longitude=bestLong;
      patPtr->altitude=bestAlt;
      patPtr->heading=bestHeading;     
      intFlag=Int_t(diffTime);

      if(thePat) delete thePat;
      thePat = new Adu5Pat(*patPtr);
      thePat->intFlag=intFlag;
      adu5PatTreeInt->Fill();

   }
   adu5PatTreeInt->AutoSave();
   fpOut->Close();
   cerr << endl;
}
