#include <cstdio>
#include <fstream>
#include <iostream>
#include <zlib.h>
#include <libgen.h>
   

#include "TTree.h"
#include "TFile.h"
#include "TSystem.h"
#include "TTreeIndex.h"

#define HACK_FOR_ROOT

#include "simpleStructs.h"
#include "SlowRate.h"


void processSlowRate();
void makeSlowRateTree(char *inName, char *outName);

//Global Variable
SlowRate *theMonHkPtr;
SlowRateFull_t theSlowRate;
TFile *theFile;
TTree *slowTree;
char rootFileName[FILENAME_MAX];
int doneInit=0;
int runNumber;
UInt_t realTime;

int main(int argc, char **argv) {
  if(argc<3) {
    std::cout << "Usage: " << basename(argv[0]) << " <file list> <outfile>" << std::endl;
    return -1;
  }
  makeSlowRateTree(argv[1],argv[2]);
  return 0;
}


void makeSlowRateTree(char *inName, char *outName) {
   strncpy(rootFileName,outName,FILENAME_MAX);

    ifstream PosFile(inName);

    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
    while(PosFile >> fileName) {
	const char *subDir = gSystem->DirName(fileName);
	const char *subSubDir = gSystem->DirName(subDir);
	const char *slowDir= gSystem->DirName(subSubDir);
	const char *houseDir = gSystem->DirName(slowDir);
	const char *runDir = gSystem->DirName(houseDir);
	const char *justRun = gSystem->BaseName(runDir);
//	std::cout << justRun << std::endl;
	sscanf(justRun,"run%d",&runNumber);


	if(counter%100==0)
	    std::cout << fileName << std::endl;
	counter++;
	gzFile infile = gzopen (fileName, "rb");    
	for(int i=0;i<1000;i++) {	
	    numBytes=gzread(infile,&theSlowRate,sizeof(SlowRateFull_t));
	    if(numBytes!=sizeof(SlowRateFull_t)) {
		error=1;
		break;
	    }
	    processSlowRate();
	}
	gzclose(infile);
//	if(error) break;
    }
    
    slowTree->AutoSave();
    theFile->Close();
    doneInit=0;
}

void processSlowRate() {
    if(!doneInit) {
	theFile = new TFile(rootFileName,"RECREATE");

	slowTree = new TTree("slowTree","Tree of Anita CPU SlowRate Stuff");
	slowTree->Branch("mon","SlowRate",&theMonHkPtr);
	doneInit=1;
    }
    if(theMonHkPtr) delete theMonHkPtr;
    theMonHkPtr = new SlowRate(runNumber,theSlowRate.unixTime,&theSlowRate);
    slowTree->Fill();                
}


