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

#include "AveragedSurfHk.h"
#include "simpleStructs.h"

void processSurfHk(int version);
void makeAveragedSurfHkTree(char *inName, char *outName);


//Global Variable
AveragedSurfHk *surfHkPtr=0;
AveragedSurfHkStruct_t theSurfHk;
AveragedSurfHkStructVer14_t theSurfHkVer14;
AveragedSurfHkStructVer13_t theSurfHkVer13;
AveragedSurfHkStructVer12_t theSurfHkVer12;
TFile *theFile;
TTree *surfhkTree;
char rootFileName[FILENAME_MAX];
int doneInit=0;
int runNumber;
UInt_t realTime;

int main(int argc, char **argv) {
  if(argc<3) {
    std::cout << "Usage: " << basename(argv[0]) << " <file list> <outfile>" << std::endl;
    return -1;
  }
  makeAveragedSurfHkTree(argv[1],argv[2]);
  return 0;
}


void makeAveragedSurfHkTree(char *inName, char *outName) {
   strncpy(rootFileName,outName,FILENAME_MAX);
   std::cout << sizeof(AveragedSurfHkStruct_t) << std::endl;
    ifstream PosFile(inName);

    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
    int firstTime=1;
    int version=VER_AVG_SURF_HK;
    GenericHeader_t gHdr;

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

      if(firstTime) {
	 //Need to work out which version this is
	 numBytes=gzread(infile,&gHdr,sizeof(GenericHeader_t));
	 if(numBytes!=sizeof(GenericHeader_t)) {
	    std::cerr << "Error reading GenericHeader_t to determine version\n";
	    exit(0);
	 }
	 gzrewind(infile);
	 if(gHdr.code != PACKET_AVG_SURF_HK) {
	    std::cerr << "not a AveragedSurfHkStruct_t\n";
	    exit(0);
	 }
	 if(gHdr.verId != VER_AVG_SURF_HK) {
	    std::cout << "Old version of AveragedSurfHkStruct_t -- " << (int)gHdr.verId
		 << std::endl;
	    switch(gHdr.verId) {
	    case 14:
	       if(gHdr.numBytes==sizeof(AveragedSurfHkStructVer14_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=14;
	       }
	       break;
	    case 13:
	       if(gHdr.numBytes==sizeof(AveragedSurfHkStructVer13_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=13;
	       }
	       break;
	    case 12:
	    case 11:
	    case 10:
	       if(gHdr.numBytes==sizeof(AveragedSurfHkStructVer12_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=12;
	       }
	       break;
	    default:
	       std::cerr << "This version is not currently supported someone needs, to update me\n";
	       exit(0);
	    }
	 }	       
	 firstTime=0;
	 std::cout << "Proceeding with version " << (int)version << " (" 
	      << VER_AVG_SURF_HK << ")\n";
      }

   
	for(int i=0;i<1000;i++) {
	   if(version==VER_AVG_SURF_HK) {
	      numBytes=gzread(infile,&theSurfHk,sizeof(AveragedSurfHkStruct_t));
	      if(numBytes!=sizeof(AveragedSurfHkStruct_t)) {
		 error=1;
		 break;
	      }
	   }
	   else {
	    int numBytesExpected=sizeof(AveragedSurfHkStruct_t);
	    switch(version) {
	    case 14:
	       numBytesExpected=sizeof(AveragedSurfHkStructVer14_t);
	       numBytes=gzread(infile,&theSurfHkVer14,numBytesExpected);
	       break;
	    case 13:
	       numBytesExpected=sizeof(AveragedSurfHkStructVer13_t);
	       numBytes=gzread(infile,&theSurfHkVer13,numBytesExpected);
	       break;
	    case 12:
	       numBytesExpected=sizeof(AveragedSurfHkStructVer12_t);
	       numBytes=gzread(infile,&theSurfHkVer12,numBytesExpected);
	       break;
	    default:
	       std::cerr << "Shouldn't ever get here\n";
	       exit(0);
	    }
	    if(numBytes!=numBytesExpected) {
	       error=1;
	       break;
	    }	 
	   }
	   processSurfHk(version);
	      //	    std::cout << theSurfHk.avgScaler[0][0] << std::endl;
	}
	gzclose(infile);
//	if(error) break;
    }
    
    surfhkTree->AutoSave();
    //    theFile->Close();
    doneInit=0;
}

void processSurfHk(int version) {
    if(!doneInit) {
	theFile = new TFile(rootFileName,"RECREATE");
	surfhkTree = new TTree("avgSurfHkTree","Tree of Anita Scalers And Thresholds");
	surfhkTree->Branch("avgsurf","AveragedSurfHk",&surfHkPtr);
   
	doneInit=1;
    }
    if(surfHkPtr) delete surfHkPtr;
    if(version==VER_AVG_SURF_HK) {
       surfHkPtr = new AveragedSurfHk(runNumber,theSurfHk.unixTime,&theSurfHk);
    }
    else {
       switch(version) {
       case 14:
	  surfHkPtr = new AveragedSurfHk(runNumber,theSurfHkVer14.unixTime,&theSurfHkVer14);
	  break;
       case 13:
	  surfHkPtr = new AveragedSurfHk(runNumber,theSurfHkVer13.unixTime,&theSurfHkVer13);
	  break;
       case 12:
	  surfHkPtr = new AveragedSurfHk(runNumber,theSurfHkVer12.unixTime,&theSurfHkVer12);
	  break;
       default:
	  std::cout << "And shouldn't ver get here\n";
	  exit(0);
       }
       
    }
			   
    surfhkTree->Fill();                
}



