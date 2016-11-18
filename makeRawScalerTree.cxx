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

#define HACK_FOR_ROOT

#include "simpleStructs.h"


void processScaler();
void makeRawScalerTree(char *inName, char *outName);


//Global Variable

SimpleScalerStruct_t theScaler;
TFile *theFile;
TTree *rawScalerTree;
char rootFileName[FILENAME_MAX];
int doneInit=0;
int runNumber;
UInt_t realTime;

int main(int argc, char **argv) {
  if(argc<3) {
    std::cout << "Usage: " << basename(argv[0]) << " <file list> <outfile>" << std::endl;
    return -1;
  }
  makeRawScalerTree(argv[1],argv[2]);
  return 0;
}


void makeRawScalerTree(char *inName, char *outName) {
   strncpy(rootFileName,outName,FILENAME_MAX);
   std::cout << sizeof(SimpleScalerStruct_t) << std::endl;
    std::ifstream PosFile(inName);

    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
    while(PosFile >> fileName) {
	const char *subDir = gSystem->DirName(fileName);
	const char *subSubDir = gSystem->DirName(subDir);
	const char *surfhkDir= gSystem->DirName(subSubDir);
	const char *houseDir = gSystem->DirName(surfhkDir);
	const char *runDir = gSystem->DirName(houseDir);
	const char *justRun = gSystem->BaseName(runDir);
//	std::cout << justRun << std::endl;
	sscanf(justRun,"run%d",&runNumber);


	if(counter%100==0)
	    std::cout << fileName << std::endl;
	counter++;
	gzFile infile = gzopen (fileName, "rb");    
	for(int i=0;i<1000;i++) {	
	    numBytes=gzread(infile,&theScaler,sizeof(SimpleScalerStruct_t));
	    if(numBytes!=sizeof(SimpleScalerStruct_t)) {
		error=1;
		break;
	    }
	    processScaler();
	}
	gzclose(infile);
//	if(error) break;
    }
    
    rawScalerTree->AutoSave();
    //    theFile->Close();
    doneInit=0;
}
//SURF4 scalers 4-7 are its FIRST antenna band coincidences with pulse.
//   Scalers 25-28 are its SECOND antenna band coincidences with pulse
//   Scaler0=num pulses, Scaler1=num coincident ANT1 L1s, Scaler2 = ANT2 L1s



Float_t pL1[10][4],pLow[10][4],pMid[10][4],pHigh[10][4],pFull[10][4],pCalc2of3[10][4],pCalcL1[10][4];
Float_t pLMF2,pMHF2,pLHF2;

Int_t l1Index[4]={1,2,31,100};
Int_t lowIndex[4]={4,24,3,96};
Int_t midIndex[4]={5,25,28,97};
Int_t highIndex[4]={6,26,29,98};
Int_t fullIndex[4]={7,27,30,99};
Int_t lmfIndex=101;
Int_t mhfIndex=102;
Int_t lhfIndex=103;

void processScaler() {
    if(!doneInit) {
	theFile = new TFile(rootFileName,"RECREATE");
	rawScalerTree = new TTree("rawScalerTree","Tree of Anita Scalers And Thresholds");
	rawScalerTree->Branch("unixTime",&theScaler.unixTime,"unixTime/I");
	rawScalerTree->Branch("unixTimeUs",&theScaler.unixTimeUs,"unixTimeUs/I");
	rawScalerTree->Branch("scaler",&theScaler.scaler[0][0],"scaler[12][32]/S");
	rawScalerTree->Branch("pL1",pL1,"pL1[10][4]/F");
	rawScalerTree->Branch("pLow",pLow,"pLow[10][4]/F");
	rawScalerTree->Branch("pMid",pMid,"pMid[10][4]/F");
	rawScalerTree->Branch("pHigh",pHigh,"pHigh[10][4]/F");
	rawScalerTree->Branch("pFull",pFull,"pFull[10][4]/F"); 
	rawScalerTree->Branch("pCalc2of3",pCalc2of3,"pCalc2of3[10][4]/F"); 
	rawScalerTree->Branch("pCalcL1",pCalcL1,"pCalcL1[10][4]/F"); 
	doneInit=1;
    }
    for(int surf=0;surf<10;surf++) {
      for(int ant=0;ant<4;ant++) {
	if(l1Index[ant]<32)
	  pL1[surf][ant]=float(theScaler.scaler[surf][l1Index[ant]])/theScaler.scaler[surf][0];
	else
	  pL1[surf][ant]=float(theScaler.extraScaler[surf][l1Index[ant]-96])/theScaler.scaler[surf][0];
	if(lowIndex[ant]<32)
	  pLow[surf][ant]=float(theScaler.scaler[surf][lowIndex[ant]])/theScaler.scaler[surf][0];
	else
	  pLow[surf][ant]=float(theScaler.extraScaler[surf][lowIndex[ant]-96])/theScaler.scaler[surf][0];
	if(midIndex[ant]<32)
	  pMid[surf][ant]=float(theScaler.scaler[surf][midIndex[ant]])/theScaler.scaler[surf][0];
	else
	  pMid[surf][ant]=float(theScaler.extraScaler[surf][midIndex[ant]-96])/theScaler.scaler[surf][0];
	if(highIndex[ant]<32)
	  pHigh[surf][ant]=float(theScaler.scaler[surf][highIndex[ant]])/theScaler.scaler[surf][0];
	else
	  pHigh[surf][ant]=float(theScaler.extraScaler[surf][highIndex[ant]-96])/theScaler.scaler[surf][0];
	if(fullIndex[ant]<32)
	  pFull[surf][ant]=float(theScaler.scaler[surf][fullIndex[ant]])/theScaler.scaler[surf][0];
	else
	  pFull[surf][ant]=float(theScaler.extraScaler[surf][fullIndex[ant]-96])/theScaler.scaler[surf][0];
	
	
	pCalc2of3[surf][ant]=pLow[surf][ant]*pMid[surf][ant]+pLow[surf][ant]*pHigh[surf][ant]+pMid[surf][ant]*pHigh[surf][ant]-2*pLow[surf][ant]*pMid[surf][ant]*pHigh[surf][ant];
	pCalcL1[surf][ant]=pCalc2of3[surf][ant]*pFull[surf][ant];
	
      }
    }
    rawScalerTree->Fill();                
}



