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

#include "RawAnitaHeader.h"

using namespace std;

void fixTriggerTimeAnita3( char *headName, char *epochName, char *outName);

#define LEAP_SECOND_CORRCTION 2

int main(int argc, char **argv) {
  if(argc<3) {
    std::cout << "Usage: " << basename(argv[0]) << " <inheadfile> <epochfile> <outheadfile>" << std::endl;
    return -1;
  }
  fixTriggerTimeAnita3(argv[1],argv[2],argv[3]);
  return 0;
}


void fixTriggerTimeAnita3(char *headName, char *epochName, char *outName) {

   RawAnitaHeader *headPtr=0;

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
   TTree *headTreeOut = new TTree("headTree",headTree->GetTitle());   
   headTreeOut->Branch("header","RawAnitaHeader",&headPtr);

   TFile *fpEpoch = TFile::Open(epochName,"UPDATE");
   TTree *epochTree = (TTree*) fpEpoch->Get("epochTree");
   Int_t epochNum=-1;
   Int_t firstEventNum;
   Int_t lastEventNum;
   Long64_t firstEntry;
   Long64_t lastEntry;
   Int_t bestDiff=0;
   UInt_t bestSecDiff=0;
   UInt_t numMatched=0;
   epochTree->SetBranchAddress("epochNum",&epochNum);
   epochTree->SetBranchAddress("firstEventNum",&firstEventNum);
   epochTree->SetBranchAddress("lastEventNum",&lastEventNum);
   epochTree->SetBranchAddress("firstEntry",&firstEntry);
   epochTree->SetBranchAddress("lastEntry",&lastEntry);
   epochTree->SetBranchAddress("bestDiff",&bestDiff);
   epochTree->SetBranchAddress("secDiff",&bestSecDiff);
   epochTree->SetBranchAddress("numMatched",&numMatched);

   

   Long64_t epochEntries=epochTree->GetEntries();
   Long64_t headEntries = headTree->GetEntries();
   Long64_t epochIndex=0;
   epochTree->GetEntry(epochIndex);
   bestSecDiff-=LEAP_SECOND_CORRCTION;
   bestDiff-=LEAP_SECOND_CORRCTION;
   Int_t lastBestDiff=bestDiff;
   if(lastBestDiff==-12) {
     lastBestDiff+=13;
     bestSecDiff+=13;
   }
   std::cout << "Start:\t" << lastBestDiff << "\t" << bestSecDiff-1.42e9 << "\n";

   
   Double_t lastOffset=0;

   Long64_t nbytes = 0, nb = 0;
   Int_t goodFlag=0;

   Double_t totalOffset=0;
   Double_t numOffset=0;

   Int_t lastGoodC3poNum=250000000;

   
   
   
   for (Long64_t jentry=0; jentry<headEntries;jentry++) {
      if(jentry%10000==0) cerr << "*";
      nb = headTree->GetEntry(jentry);   nbytes += nb;

      if(jentry>lastEntry && epochIndex<epochEntries) {
	epochIndex++;
	epochTree->GetEntry(epochIndex);
	//Insert check here for the one broken file
	bestSecDiff-=LEAP_SECOND_CORRCTION;
	bestDiff-=LEAP_SECOND_CORRCTION;
	std::cout << bestDiff << "\t" << bestSecDiff-1.42e9 << "\n";
	goodFlag=0;
	if(numMatched>0.1*(lastEntry-firstEntry)) goodFlag=1;
	
	if(goodFlag==0 || TMath::Abs(bestDiff-lastBestDiff)>5) {
	  //Bad data alert
	  std::cout << "Broken something: " << lastBestDiff << " followed by " << bestDiff << "\t" << firstEntry << " using previous entry\n";
	  goodFlag=0;
	  if(headPtr->run==373 && firstEntry==327546) {
	    bestSecDiff+=Int_t(4)-bestDiff;
	    bestDiff=Int_t(4);
	  }
	  else {
	    bestSecDiff+=lastBestDiff-bestDiff;
	    bestDiff=lastBestDiff;
	  }
	}
	else {
	  lastBestDiff=bestDiff;
	}
      }

      Int_t thisC3po=headPtr->c3poNum;
      if(TMath::Abs(thisC3po-250000000)>1e6) {
	thisC3po=lastGoodC3poNum;
      }
      else {
	lastGoodC3poNum=thisC3po;
      }
      
      if(headPtr->triggerTimeNs>=1000000000) {
	if(headPtr->trigTime<thisC3po) {
	  headPtr->triggerTimeNs=Int_t(1e9*(Double_t(headPtr->trigTime)/thisC3po));
	}
	else {
	  //Have trigTime > 1s
	  headPtr->ppsNum++;
	  headPtr->trigTime-=thisC3po;
	  headPtr->triggerTimeNs=Int_t(1e9*(Double_t(headPtr->trigTime)/thisC3po));
	}
      }
      //      if(headPtr->triggerTimeNs>1e9*Double_t(headPtr->trigTime)/thisC3po) {
      headPtr->triggerTimeNs=Int_t(1e9*(Double_t(headPtr->trigTime)/thisC3po));
	//      }
	
      
      headPtr->triggerTime=headPtr->ppsNum+bestSecDiff;
      headPtr->goodTimeFlag=goodFlag;
      Double_t offset=Double_t(headPtr->triggerTime+1e-9*headPtr->triggerTimeNs)-Double_t(headPtr->payloadTime+1e-6*headPtr->payloadTimeUs);
      if(jentry>400300 && jentry<400400) {
	std::cout << jentry << "\t" << headPtr->eventNumber << "\t" << offset << "\t" << lastOffset << "\t" << totalOffset/numOffset<< "\t" << totalOffset << "\t" <<numOffset << "\t" << headPtr->ppsNum << "\n";
      }

      
      if(jentry>0 && (offset-(totalOffset/numOffset))>0.5 && headPtr->ppsNum>0) {
	  if(jentry>400300 && jentry<400400) {
	    std::cout << "Will try decrementing ppsNum: " << headPtr->ppsNum << "\n";
	  }
	headPtr->ppsNum--;
	headPtr->triggerTime=headPtr->ppsNum+bestSecDiff;
	headPtr->goodTimeFlag=0;
	offset=Double_t(headPtr->triggerTime+1e-9*headPtr->triggerTimeNs)-Double_t(headPtr->payloadTime+1e-6*headPtr->payloadTimeUs);
	
	if(jentry>400300 && jentry<400400) {
	  std::cout << "New: " << jentry << "\t" << headPtr->eventNumber << "\t" << offset << "\t" << lastOffset << "\t" << totalOffset/numOffset<< "\t" << totalOffset << "\t" <<numOffset << "\t" << headPtr->ppsNum << "\n";
	}

	//Give it a second go... but no more
	if(jentry>0 && (offset-(totalOffset/numOffset))>0.5 && headPtr->ppsNum>0) {
	  if(jentry>400300 && jentry<400400) {
	    std::cout << "Will try decrementing ppsNum: " << headPtr->ppsNum << "\n";
	  }
	  headPtr->ppsNum--;
	  headPtr->triggerTime=headPtr->ppsNum+bestSecDiff;
	  headPtr->goodTimeFlag=0;
	  offset=Double_t(headPtr->triggerTime+1e-9*headPtr->triggerTimeNs)-Double_t(headPtr->payloadTime+1e-6*headPtr->payloadTimeUs);
	}
	
      }
      totalOffset+=offset;
      numOffset++;
      

      headTreeOut->Fill();      
	 lastOffset=offset;

   }
   headTreeOut->AutoSave();
   fpOut->Close();
   cerr << endl;
}
