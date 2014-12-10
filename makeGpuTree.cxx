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
#include "GpuPowerSpectra.h"


void processGpu();
void makeGpuTree(char *inName, char *outName);

//Global Variable
GpuPowerSpectra *theGpuPtr;
GpuPhiSectorPowerSpectrumStruct_t theGpu;
TFile *theFile;
TTree *gpuTree;
char rootFileName[FILENAME_MAX];
int doneInit=0;
int runNumber;
UInt_t realTime;

int main(int argc, char **argv) {
  if(argc<3) {
    std::cout << "Usage: " << basename(argv[0]) << " <file list> <outfile>" << std::endl;
    return -1;
  }
  makeGpuTree(argv[1],argv[2]);
  return 0;
}


void makeGpuTree(char *inName, char *outName) {
   strncpy(rootFileName,outName,FILENAME_MAX);

    ifstream PosFile(inName);

    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
    while(PosFile >> fileName) {
	const char *subDir = gSystem->DirName(fileName);
	const char *subSubDir = gSystem->DirName(subDir);
	const char *gpuDir= gSystem->DirName(subSubDir);
	const char *houseDir = gSystem->DirName(gpuDir);
	const char *runDir = gSystem->DirName(houseDir);
	const char *justRun = gSystem->BaseName(runDir);
//	std::cout << justRun << std::endl;
	sscanf(justRun,"run%d",&runNumber);


	if(counter%100==0)
	    std::cout << fileName << std::endl;
	counter++;
	gzFile infile = gzopen (fileName, "rb");    
	for(int i=0;i<1000;i++) {	
	    numBytes=gzread(infile,&theGpu,sizeof(GpuPhiSectorPowerSpectrumStruct_t));
	    if(numBytes!=sizeof(GpuPhiSectorPowerSpectrumStruct_t)) {
		error=1;
		break;
	    }
	    processGpu();
	}
	gzclose(infile);
//	if(error) break;
    }
    
    gpuTree->AutoSave();
    theFile->Close();
    doneInit=0;
}

void processGpu() {
    if(!doneInit) {
	theFile = new TFile(rootFileName,"RECREATE");

	gpuTree = new TTree("gpuTree","Tree of Anita GPU Stuff");
	gpuTree->Branch("gpu","GpuPowerSpectra",&theGpuPtr);
	doneInit=1;
    }
    if(theGpuPtr) delete theGpuPtr;
    theGpuPtr = new GpuPowerSpectra(runNumber,theGpu.unixTimeFirstEvent,&theGpu);
    gpuTree->Fill();                
}


