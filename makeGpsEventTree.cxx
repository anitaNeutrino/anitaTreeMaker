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

using namespace std;

void makeGpsEventTree(char *inName, char *headName, char *outName);


int main(int argc, char **argv) {
  if(argc<3) {
    std::cout << "Usage: " << basename(argv[0]) << " <inhkfile> <inheadfile> <outhkfile>" << std::endl;
    return -1;
  }
  makeGpsEventTree(argv[1],argv[2],argv[3]);
  return 0;
}


void makeGpsEventTree(char *inName,char *headName, char *outName) {

  Adu5Pat *patPtr=0;
  RawAnitaHeader *headPtr=0;

   TFile *fpIn = new TFile(inName);
   if(!fpIn) {
      std::cerr << "Couldn't open " << inName << "\n";
      return;
   }  
   TTree *adu5PatTree = (TTree*) fpIn->Get("adu5PatTree");
   if(!adu5PatTree) {
      std::cerr << "Couldn't get hkTree from " << inName << "\n";
      return;
   }      
   TTree *adu5bPatTree = (TTree*) fpIn->Get("adu5bPatTree");
   if(!adu5bPatTree) {
      std::cerr << "Couldn't get hkTree from " << inName << "\n";
      return;
   }      
   adu5PatTree->SetBranchAddress("pat",&patPtr);   
   adu5bPatTree->SetBranchAddress("pat",&patPtr);   
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
   for (Long64_t jentry=0; jentry<headEntries;jentry++) {
      if(jentry%10000==0) cerr << "*";
      nb = headTree->GetEntry(jentry);   nbytes += nb;
      
      Long64_t adu5aEntry=adu5PatTree->GetEntryNumberWithBestIndex(headPtr->triggerTime,
							   headPtr->triggerTimeNs/1000);
      if(adu5aEntry>=0 && adu5aEntry<nentries) {
	 adu5PatTree->GetEntry(adu5aEntry);
      }
      else {
	 std::cerr << "Invalid entry " << adu5aEntry << "\t" << headPtr->triggerTime << "\t" << headPtr->triggerTimeNs << "\n";
	 continue;
      }
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
