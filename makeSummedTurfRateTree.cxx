#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <zlib.h>
#include <libgen.h>      

#include "TTree.h"
#include "TFile.h"
#include "TSystem.h"
#include "TTreeIndex.h"

#include "SummedTurfRate.h"
#include "simpleStructs.h"

using namespace std;

void processTurfRate(int version);
void makeSummedTurfRateTree(char *inName, char *outName);

//Global Variable
SummedTurfRateStruct_t theSummedTurfRate;
SummedTurfRateStructVer40_t theSummedTurfRateVer40;
TFile *theFile;
TTree *sumTurfRateTree=0;
SummedTurfRate *theSummedTurfRateClass;
char rootFileName[FILENAME_MAX];
int doneInit=0;
int runNumber;
UInt_t realTime;


int main(int argc, char **argv) {
  if(argc<3) {
    std::cout << "Usage: " << basename(argv[0]) << " <file list> <outfile>" << std::endl;
    return -1;
  }
  makeSummedTurfRateTree(argv[1],argv[2]);
  return 0;
}


void makeSummedTurfRateTree(char *inName, char *outName) {
   doneInit=0;
   strncpy(rootFileName,outName,FILENAME_MAX);
    std::ifstream PosFile(inName);
    
    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
    
    int firstTime=1;
    GenericHeader_t gHdr;
    int version=VER_SUM_TURF_RATE;

    while(PosFile >> fileName) {
	const char *subDir = gSystem->DirName(fileName);
	const char *subSubDir = gSystem->DirName(subDir);
	const char *turfRateDir= gSystem->DirName(subSubDir);
	const char *houseDir = gSystem->DirName(turfRateDir);
	const char *runDir = gSystem->DirName(houseDir);
	const char *justRun = gSystem->BaseName(runDir);
//	std::cout << justRun << endl;
	sscanf(justRun,"run%d",&runNumber);


	if(counter%100==0)
	    std::cout << fileName << std::endl;
	counter++;
	gzFile infile = gzopen (fileName, "rb");    

      if(firstTime) {
	 //Need to work out which version this is
	 numBytes=gzread(infile,&gHdr,sizeof(GenericHeader_t));
	 if(numBytes!=sizeof(GenericHeader_t)) {
	    std::cerr << "Error reading GenericHeader_t to determine version\n";
	    exit(0);
	 }
	 gzrewind(infile);
	 if(gHdr.code != PACKET_SUM_TURF_RATE) {
	    std::cerr << "not a SummedTurfRateStruct_t\n";
	    exit(0);
	 }

	 if(gHdr.verId != VER_SUM_TURF_RATE) {	    
	    std::cout << "Got version:\t" << version << " current " << VER_SUM_TURF_RATE << "\n";
	    switch(gHdr.verId) {
	    case 40:
	      if(gHdr.numBytes==sizeof(SummedTurfRateStructVer40_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=40;
	      }
	    default:
	      std::cerr << "This version is not currently supported someone needs, to update me\n";
	      exit(0);
	    }


	 }
	 firstTime=0;
      }

	for(int i=0;i<1000;i++) {
	   if(version==VER_SUM_TURF_RATE) {
	      numBytes=gzread(infile,&theSummedTurfRate,sizeof(SummedTurfRateStruct_t));
	      if(numBytes!=sizeof(SummedTurfRateStruct_t)) {
		 error=1;
		 break;
	      }
	   }
	   else {
	    int numBytesExpected=sizeof(SummedTurfRateStruct_t);
	    switch(version) {
	    case 40:
	       numBytesExpected=sizeof(SummedTurfRateStructVer40_t);
	       numBytes=gzread(infile,&theSummedTurfRateVer40,numBytesExpected);
	       break;
	    default:
	       std::cerr << "Shouldn't ever get here\n";
	       exit(0);
	    }	
	   }
	   processTurfRate(version);
	}
	gzclose(infile);
//	if(error) break;
    }
    
    sumTurfRateTree->AutoSave();
    theFile->Close();
    doneInit=0;
}

void processTurfRate(int version) {
    if(!doneInit) {
	theFile = new TFile(rootFileName,"RECREATE");

	sumTurfRateTree = new TTree("sumTurfRateTree","Tree of Anita TURF Trigger Rates");
	sumTurfRateTree->Branch("sumturf","SummedTurfRate",&theSummedTurfRateClass);       	

	doneInit=1;
	std::cout << sumTurfRateTree << "\n";
    }
    cout << theSummedTurfRate.unixTime
    	 << "\t" << theSummedTurfRate.gHdr.code
    	 << endl;
    if(theSummedTurfRateClass) 
       delete theSummedTurfRateClass;
    if(version == VER_SUM_TURF_RATE) {
       theSummedTurfRateClass = new SummedTurfRate(runNumber,
						   theSummedTurfRate.unixTime,
						   &theSummedTurfRate);
    }
    else {
      switch(version) {
       case 40:
	  theSummedTurfRateClass = new SummedTurfRate(runNumber,theSummedTurfRateVer40.unixTime,&theSummedTurfRateVer40);
	  break;
       default:
	  std::cout << "And shouldn't ever get here\n";
	  exit(0);
      }
    }
    sumTurfRateTree->Fill();
    //    delete theSummedTurfRateClass;
}




