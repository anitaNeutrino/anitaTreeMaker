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

#include "PrettyAnitaHk.h"
#include "CalibratedHk.h"
#include "RawAnitaHeader.h"

using namespace std;

void makePrettyHkTree(char *inName, char *headName, char *outName);


int main(int argc, char **argv) {
  if(argc<3) {
    std::cout << "Usage: " << basename(argv[0]) << " <inhkfile> <inheadfile> <outhkfile>" << std::endl;
    return -1;
  }
  makePrettyHkTree(argv[1],argv[2],argv[3]);
  return 0;
}


void makePrettyHkTree(char *inName,char *headName, char *outName) {

   CalibratedHk *hkPtr=0;
   RawAnitaHeader *headPtr=0;

   TFile *fpIn = new TFile(inName);
   if(!fpIn) {
      std::cerr << "Couldn't open " << inName << "\n";
      return;
   }  
   TTree *hkTree = (TTree*) fpIn->Get("hkTree");
   if(!hkTree) {
      std::cerr << "Couldn't get hkTree from " << inName << "\n";
      return;
   }      
   hkTree->SetBranchAddress("hk",&hkPtr);   
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
   TTree *prettyHkTree = new TTree("prettyHkTree","Tree of Housekeeping Values");
   PrettyAnitaHk *thePrettyHk = new PrettyAnitaHk();
   prettyHkTree->Branch("hk","PrettyAnitaHk",&thePrettyHk);

 

   

   Long64_t headEntries = headTree->GetEntries();

   Long64_t nentries = hkTree->GetEntries();
   hkTree->BuildIndex("payloadTime","payloadTimeUs");

   Long64_t nbytes = 0, nb = 0;
   Int_t intFlag;
   for (Long64_t jentry=0; jentry<headEntries;jentry++) {
      if(jentry%10000==0) cerr << "*";
      nb = headTree->GetEntry(jentry);   nbytes += nb;
      
      Long64_t hkEntry=hkTree->GetEntryNumberWithBestIndex(headPtr->triggerTime,
							   headPtr->triggerTimeNs/1000);
      if(hkEntry>=0 && hkEntry<nentries) {
	 hkTree->GetEntry(hkEntry);
      }
      else {
	 std::cerr << "Invalid entry " << hkEntry << "\t" << headPtr->triggerTime << "\t" << headPtr->triggerTimeNs << "\n";
	 continue;
      }
      intFlag=headPtr->triggerTime-hkPtr->realTime;

      if(thePrettyHk) delete thePrettyHk;
      thePrettyHk = new PrettyAnitaHk(hkPtr,intFlag);
      prettyHkTree->Fill();

   }
   prettyHkTree->AutoSave();
   fpOut->Close();
   cerr << endl;
}
