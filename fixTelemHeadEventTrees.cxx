#include <iostream>

#include "TFile.h"
#include "TTree.h"
#include "TMath.h"

#include "RawAnitaHeader.h"
#include "RawAnitaEvent.h"


void fixStuff(char *inputdir, int run);

int main(int argc, char **argv) {
  if(argc<3) {
    std::cout << "Usage: " << basename(argv[0]) << " <data dir> <run number>" << std::endl;
    return -1;
  }
  int run =  atoi(argv[2]);
  fixStuff(argv[1],run);
  return 0;
}



void fixStuff(char *inputdir, int run){

  TFile *fHead = new TFile(Form("%s/run%i/headFile%i.root",  inputdir, run, run));
  TFile *fEvt  = new TFile(Form("%s/run%i/eventFile%i.root", inputdir, run, run));

  TTree *oldHeadTree = (TTree*)fHead->Get("headTree");
  TTree *oldEvtTree  = (TTree*)fEvt ->Get("eventTree");

  RawAnitaHeader *oldHead  = NULL;
  RawAnitaEvent  *oldEvt   = NULL;

  oldHeadTree->SetBranchAddress("header", &oldHead);
  oldEvtTree ->SetBranchAddress("event",  &oldEvt );

  oldHeadTree->BuildIndex("eventNumber");
  oldEvtTree ->BuildIndex("eventNumber");

  TTree *newHeadTree = new TTree();
  TTree *newEvtTree  = new TTree();
  TTree *badEvtTree  = new TTree();

  RawAnitaHeader *newHead  = NULL;
  RawAnitaEvent  *newEvt   = NULL;
  RawAnitaEvent  *badEvt   = NULL;

  newHeadTree->Branch("header", &newHead);
  newEvtTree ->Branch("event",  &newEvt );
  badEvtTree ->Branch("event",  &badEvt );

  oldHeadTree->GetEntry(0);
  oldEvtTree->GetEntry(0);

  unsigned long evtNumHead = oldHead->eventNumber;
  unsigned long evtNumEvt  = oldHead->eventNumber;

  unsigned long entry = TMath::Max(evtNumHead, evtNumEvt);
  int nMax  = TMath::Max(oldEvtTree ->GetEntries(), oldHeadTree->GetEntries());
  int count = 0;

  while (count < nMax){

    int it1 =  oldHeadTree->GetEntryWithIndex(entry);
    int it2 =  oldEvtTree->GetEntryWithIndex(entry);

    if ( it1!=-1 && it2!=-1 ){
      if (oldHead->eventNumber==oldHead->eventNumber){
	// cout << "Hurray " << oldHead->eventNumber << " " << oldHead->eventNumber << " " << it1 << " " << it2 << std::endl;
	bool good = true;
	for (int i=0;i<108;i++) if (oldEvt->xMax[i]==0) good=false;
	if (!good){
	  badEvt = oldEvt;
	  badEvtTree->Fill();
	}else{
	  newHead = oldHead;
	  newEvt  = oldEvt;
	  newHeadTree->Fill();
	  newEvtTree ->Fill();
	}
      }
    }

    entry++;
    count++;
  }


  TFile *foutHead = new TFile(Form("%s/run%i/eventHeadFile%iUpdated.root", inputdir, run, run), "recreate");
  newHeadTree->Write("headTree");
  TFile *foutEvt  = new TFile(Form("%s/run%i/eventFile%iUpdated.root", inputdir, run, run), "recreate");
  newEvtTree ->Write("eventTree");
  TFile *foutBad  = new TFile(Form("%s/run%i/eventFile%iBad.root", inputdir, run, run), "recreate");
  badEvtTree ->Write("eventTree");

  foutHead->Write();
  foutEvt->Write();
  foutBad->Write();

  foutHead->Close();
  foutEvt->Close();
  foutBad->Close();

  fHead->Close();
  fEvt->Close();

}
