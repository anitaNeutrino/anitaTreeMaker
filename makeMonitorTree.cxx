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
#include "MonitorHk.h"


void processMonitor();
void makeMonitorTree(char *inName, char *outName);

//Global Variable
MonitorHk *theMonHkPtr;
MonitorStruct_t theMonitor;
TFile *theFile;
TTree *monitorTree;
char rootFileName[FILENAME_MAX];
int doneInit=0;
int runNumber;
UInt_t realTime;

int main(int argc, char **argv) {
  if(argc<3) {
    std::cout << "Usage: " << basename(argv[0]) << " <file list> <outfile>" << std::endl;
    return -1;
  }
  makeMonitorTree(argv[1],argv[2]);
  return 0;
}


void makeMonitorTree(char *inName, char *outName) {
   strncpy(rootFileName,outName,FILENAME_MAX);

   std::ifstream PosFile(inName);

    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
    while(PosFile >> fileName) {
	const char *subDir = gSystem->DirName(fileName);
	const char *subSubDir = gSystem->DirName(subDir);
	const char *monitorDir= gSystem->DirName(subSubDir);
	const char *houseDir = gSystem->DirName(monitorDir);
	const char *runDir = gSystem->DirName(houseDir);
	const char *justRun = gSystem->BaseName(runDir);
//	std::cout << justRun << std::endl;
	sscanf(justRun,"run%d",&runNumber);


	if(counter%100==0)
	    std::cout << fileName << std::endl;
	counter++;
	gzFile infile = gzopen (fileName, "rb");    
	for(int i=0;i<1000;i++) {	
	    numBytes=gzread(infile,&theMonitor,sizeof(MonitorStruct_t));
	    if(numBytes!=sizeof(MonitorStruct_t)) {
		error=1;
		break;
	    }
	    processMonitor();
	}
	gzclose(infile);
//	if(error) break;
    }
    
    monitorTree->AutoSave();
    theFile->Close();
    doneInit=0;
}

void processMonitor() {
    if(!doneInit) {
	theFile = new TFile(rootFileName,"RECREATE");

	monitorTree = new TTree("monitorTree","Tree of Anita CPU Monitor Stuff");
	monitorTree->Branch("mon","MonitorHk",&theMonHkPtr);
	doneInit=1;
    }
    if(theMonHkPtr) delete theMonHkPtr;
    theMonHkPtr = new MonitorHk(runNumber,theMonitor.unixTime,&theMonitor);
    monitorTree->Fill();                
}


