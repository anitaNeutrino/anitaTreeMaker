#include <iostream>
#include <vector>
#include <math.h>
#include <stdio.h>

#include "TFile.h"
#include "TTree.h"
#include "TChain.h"

#include "Adu5Pat.h"
#include "SurfHk.h"
#include "AveragedSurfHk.h"
#include "RawAnitaHeader.h"
#include "simpleStructs.h"

using namespace std;

void makeConcatanatedFiles(char *baseRunDir, char *outputDir);

int main(int argc, char**argv) {
  if(argc<3) {
    cerr << "Usage:\n\t" << argv[0] << "\t<base run dir> <output dir>" << endl;
    return -1;
  }
  makeConcatanatedFiles(argv[1],argv[2]);
  return 0;
}



void makeConcatanatedFiles(char *baseRunDir, char *outputDir)
{
  Int_t starEvery=1;
  //Make concatanted gpsFile
 //  TChain *adu5Input = new TChain("adu5PatTree");
//   char gpsName[FILENAME_MAX];
//   for(int run=12;run<=262;run++) {
//     sprintf(gpsName,"%s/run%d/gpsFile%d.root",baseRunDir,run,run);
//     adu5Input->Add(gpsName);
//   }

//   Adu5Pat *thePatPtr=0;
//   adu5Input->SetBranchAddress("pat",&thePatPtr);

//   sprintf(gpsName,"%s/gpsFile.root",outputDir);
//   TFile *fpGpsOut = new TFile(gpsName,"RECREATE");
//   TTree *adu5PatTree = new TTree("adu5PatTree","Tree of ADU5 Position and Attitude Info");
//   adu5PatTree->Branch("pat","Adu5Pat",&thePatPtr);

//   Long64_t numEntries=adu5Input->GetEntries();
//   std::cout << "there are " << numEntries << " entries\n";
//   starEvery=numEntries/100;
//   if(starEvery==0) starEvery++;
//   for(Long64_t entry=0;entry<numEntries;entry++) {
//     if(entry%starEvery==0)
//       std::cerr << "*";
//     adu5Input->GetEntry(entry);
//     adu5PatTree->Fill();    
//   }
//   std::cerr << "\n";
//   adu5PatTree->AutoSave();
//   fpGpsOut->Close();


  TChain *surfInput = new TChain("surfHkTree");
  char surfHkName[FILENAME_MAX];
  for(int run=12;run<=262;run++) {
    sprintf(surfHkName,"%s/run%d/surfHkFile%d.root",baseRunDir,run,run);
    surfInput->Add(surfHkName);
  }

  SurfHk *theSurfHkPtr=0;
  surfInput->SetBranchAddress("surf",&theSurfHkPtr);

  sprintf(surfHkName,"%s/surfHkFile.root",outputDir);
  TFile *fpSurfOut = new TFile(surfHkName,"RECREATE");
  TTree *surfHkTree = new TTree("surfHkTree","Tree of ANITA Scalers and Thresholds");
  surfHkTree->Branch("surf","SurfHk",&theSurfHkPtr);

  Long64_t numSurfEntries=surfInput->GetEntries();
  std::cout << "there are " << numSurfEntries << " entries\n";
  starEvery=numSurfEntries/100;
  if(starEvery==0) starEvery++;
  for(Long64_t entry=0;entry<numSurfEntries;entry++) {
    if(entry%starEvery==0)
      std::cerr << "*";
    surfInput->GetEntry(entry);
    surfHkTree->Fill();    
  }
  std::cerr << "\n";
  surfHkTree->AutoSave();
  fpSurfOut->Close();


  TChain *avgsurfInput = new TChain("avgSurfHkTree");
  char avgSurfHkName[FILENAME_MAX];
  for(int run=12;run<=262;run++) {
    sprintf(avgSurfHkName,"%s/run%d/avgSurfHkFile%d.root",baseRunDir,run,run);
    avgsurfInput->Add(avgSurfHkName);
  }

  AveragedSurfHk *theAvgSurfHkPtr=0;
  avgsurfInput->SetBranchAddress("avgsurf",&theAvgSurfHkPtr);

  sprintf(avgSurfHkName,"%s/avgSurfHkFile.root",outputDir);
  TFile *fpAvgsurfOut = new TFile(avgSurfHkName,"RECREATE");
  TTree *avgSurfHkTree = new TTree("avgSurfHkTree","Tree of ANITA Scalers and Thresholds");
  avgSurfHkTree->Branch("avgsurf","AveragedSurfHk",&theAvgSurfHkPtr);

  Long64_t numAvgSurfEntries=avgsurfInput->GetEntries();
  std::cout << "there are " << numAvgSurfEntries << " entries\n";
  starEvery=numAvgSurfEntries/100;
  if(starEvery==0) starEvery++;
  for(Long64_t entry=0;entry<numAvgSurfEntries;entry++) {
    if(entry%starEvery==0)
      std::cerr << "*";
    avgsurfInput->GetEntry(entry);
    avgSurfHkTree->Fill();    
  }
  std::cerr << "\n";
  avgSurfHkTree->AutoSave();
  fpAvgsurfOut->Close();




  TChain *headInput = new TChain("headTree");
  char headName[FILENAME_MAX];
  for(int run=12;run<=262;run++) {
    sprintf(headName,"%s/run%d/headFile%d.root",baseRunDir,run,run);
    headInput->Add(headName);
  }

  RawAnitaHeader *theRawAnitaHeaderPtr=0;
  headInput->SetBranchAddress("header",&theRawAnitaHeaderPtr);

  sprintf(headName,"%s/headFile.root",outputDir);
  TFile *fpHeadOut = new TFile(headName,"RECREATE");
  TTree *headTree = new TTree("headTree","Tree of ANITA Event headers");
  headTree->Branch("header","RawAnitaHeader",&theRawAnitaHeaderPtr);

  Long64_t numHeadEntries=headInput->GetEntries();
  std::cout << "there are " << numHeadEntries << " entries\n";
  starEvery=numHeadEntries/100;
  if(starEvery==0) starEvery++;
  for(Long64_t entry=0;entry<numHeadEntries;entry++) {
    if(entry%starEvery==0)
      std::cerr << "*";
    headInput->GetEntry(entry);
    headTree->Fill();    
  }
  std::cerr << "\n";
  headTree->AutoSave();
  fpHeadOut->Close();


}

