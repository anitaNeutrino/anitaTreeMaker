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
#include "RawSSHk.h"
#include "CalibratedSSHk.h"


void processHk();
void processHkCal();
void calcSlopesAndIntercepts();
void calibrateHk();
void makeHkTree(char *calInName, char *rawInName, char *outName);
int guessCode(SSHkDataStruct_t *hkPtr);

//Global Variable
CalibratedSSHk *goodHkPtr=0;
RawSSHk   *rawHkPtr=0;
RawSSHk   *calHkPtr=0;
RawSSHk   *avzHkPtr=0;
SSHkDataStruct_t theHk;
TFile *theFile;
TTree *hkTree=0;
TTree *hkCalTree=0;
TTree *hkAvzTree=0;
char rootFileName[FILENAME_MAX];
int doneInit=0;
int doneInitCal=0;
int runNumber;
UInt_t realTime;



int main(int argc, char **argv) {
  if(argc<4) {
    std::cout << "Usage: " << basename(argv[0]) << " <cal file list> <raw file list> <outfile>" << std::endl;
    return -1;
  }
  makeHkTree(argv[1],argv[2],argv[3]);
  return 0;
}


void makeHkTree(char *calInName, char *rawInName, char *outName) {
   strncpy(rootFileName,outName,FILENAME_MAX);
   {
      std::ifstream PosFile(calInName);
      
      int numBytes=0;
      char fileName[180];
      int error=0;
      int counter=0;
      while(PosFile >> fileName) {
	 const char *subDir = gSystem->DirName(fileName);
	 const char *subSubDir = gSystem->DirName(subDir);
	 const char *rawDir= gSystem->DirName(subSubDir);
	 const char *hkDir = gSystem->DirName(rawDir);
	 const char *houseDir = gSystem->DirName(hkDir);
	 const char *runDir = gSystem->DirName(houseDir);
	 const char *justRun = gSystem->BaseName(runDir);
	 //	std::cout << justRun << std::endl;
	 sscanf(justRun,"run%d",&runNumber);
	 
	 
	 if(counter%100==0)
	    std::cout << fileName << std::endl;
	    counter++;
	    gzFile infile = gzopen (fileName, "rb");    
	    for(int i=0;i<1000;i++) {	
	       numBytes=gzread(infile,&theHk,sizeof(SSHkDataStruct_t));
	       if(numBytes!=sizeof(SSHkDataStruct_t)) {
		  error=1;
		  break;
	       }
	       processHkCal();
	    }
	    gzclose(infile);
	    //	if(error) break;
      }
      if(hkCalTree)
	 hkCalTree->AutoSave();
      if(hkAvzTree)
	 hkAvzTree->AutoSave();
      
   }
   {
      
      std::ifstream PosFile(rawInName);
	
	int numBytes=0;
	char fileName[180];
	int error=0;
	int counter=0;
	while(PosFile >> fileName) {
	    const char *subDir = gSystem->DirName(fileName);
	    const char *subSubDir = gSystem->DirName(subDir);
	    const char *rawDir= gSystem->DirName(subSubDir);
	    const char *hkDir = gSystem->DirName(rawDir);
	    const char *houseDir = gSystem->DirName(hkDir);
	    const char *runDir = gSystem->DirName(houseDir);
	    const char *justRun = gSystem->BaseName(runDir);
//	std::cout << justRun << std::endl;
	    sscanf(justRun,"run%d",&runNumber);
	    

	    if(counter%100==0)
		std::cout << fileName << std::endl;
	    counter++;
	    gzFile infile = gzopen (fileName, "rb");    
	    for(int i=0;i<1000;i++) {	
		numBytes=gzread(infile,&theHk,sizeof(SSHkDataStruct_t));
		if(numBytes!=sizeof(SSHkDataStruct_t)) {
		    error=1;
		    break;
		}
		processHk();
	    }
	    gzclose(infile);
//	if(error) break;
	}
	if(hkTree) 
	   hkTree->AutoSave();
	//	theFile->Close();
    }
    doneInit=0;
    doneInitCal=0;
}


void processHk() {
    static Long64_t currentCalEntry=-1;
    static Long64_t currentAvzEntry=-1;
    if(!doneInit) {
	if(!theFile) 
	    theFile = new TFile(rootFileName,"UPDATE");

	hkTree = new TTree("hkTree","Tree of Anita Housekeepings");
	hkTree->Branch("hk","CalibratedSSHk",&goodHkPtr);	
	if(hkCalTree) 
	   hkCalTree->BuildIndex("payloadTime","payloadTimeUs");
	if(hkAvzTree)
	   hkAvzTree->BuildIndex("payloadTime","payloadTimeUs");
	doneInit=1;
    }

    if(hkCalTree) {
       Long64_t bestCalEntry=
	  hkCalTree->GetEntryNumberWithBestIndex(theHk.unixTime,theHk.unixTimeUs);
       if(bestCalEntry<0) bestCalEntry=0;
       if(bestCalEntry!=currentCalEntry) {
	  currentCalEntry=bestCalEntry;
	  hkCalTree->GetEntry(currentCalEntry);
       }
    }
    if(hkAvzTree) {
       Long64_t bestAvzEntry=
	  hkAvzTree->GetEntryNumberWithBestIndex(theHk.unixTime,theHk.unixTimeUs);
       if(bestAvzEntry<0) bestAvzEntry=0;
       if(bestAvzEntry!=currentAvzEntry) {
	  currentAvzEntry=bestAvzEntry;
	  hkAvzTree->GetEntry(currentAvzEntry);
       }
    }

    if(theHk.ip320.code==0) {
       theHk.ip320.code=(AnalogueCode_t)guessCode(&theHk);
    }


    if(rawHkPtr) delete rawHkPtr;
    rawHkPtr = new RawSSHk(runNumber,theHk.unixTime,&theHk);	
    if(goodHkPtr) delete goodHkPtr;
    if(hkCalTree && hkAvzTree) 
       goodHkPtr = new CalibratedSSHk(rawHkPtr,avzHkPtr,calHkPtr);    
    else 
       goodHkPtr = new CalibratedSSHk(rawHkPtr,0,0);
    hkTree->Fill();                
}



void processHkCal() {
    if(!doneInitCal) {
	theFile = new TFile(rootFileName,"RECREATE");

	hkAvzTree = new TTree("hkAvzTree","Tree of Anita Housekeeping Calibration");
	hkAvzTree->Branch("hk","RawSSHk",&avzHkPtr);
	hkCalTree = new TTree("hkCalTree","Tree of Anita Housekeeping Calibration");
	hkCalTree->Branch("hk","RawSSHk",&calHkPtr);

	doneInitCal=1;
    }
    if(theHk.ip320.code==0) {
       theHk.ip320.code=(AnalogueCode_t)guessCode(&theHk);
    }
    if(theHk.ip320.code==IP320_CAL) {
      if(calHkPtr) delete calHkPtr;
      calHkPtr = new RawSSHk(runNumber,theHk.unixTime,&theHk);	
      hkCalTree->Fill();                
    }
    else if(theHk.ip320.code==IP320_AVZ) {
      if(avzHkPtr) delete avzHkPtr;
      avzHkPtr = new RawSSHk(runNumber,theHk.unixTime,&theHk);	
      hkAvzTree->Fill(); 
    }
    
}



int guessCode(SSHkDataStruct_t *hkPtr) {
  float mean=0;
  for(int i=0;i<CHANS_PER_IP320;i++) {
     //     std::cout << i << "\t" << ((hkPtr->ip320.board.data[i])>>4) << "\n";
     mean+=((hkPtr->ip320.board.data[i])>>4);
  }
  mean/=CHANS_PER_IP320;
  //  std::cout << "Mean: " << mean << "\n";
  if(mean>4000) return IP320_CAL;
  if(mean<2050) return IP320_AVZ;


  return IP320_RAW;
}

