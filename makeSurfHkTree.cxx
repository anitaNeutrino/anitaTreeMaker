#include <cstdio>
#include <cstdlib>
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

#include "SurfHk.h"
#include "simpleStructs.h"

void processSurfHk(int version);
void makeSurfHkTree(char *inName, char *outName);


//Global Variable
SurfHk *surfHkPtr=0;
FullSurfHkStruct_t theSurfHk;
FullSurfHkStructVer14_t theSurfHkVer14;
FullSurfHkStructVer13_t theSurfHkVer13;
FullSurfHkStructVer12_t theSurfHkVer12;
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
  makeSurfHkTree(argv[1],argv[2]);
  return 0;
}


void makeSurfHkTree(char *inName, char *outName) {
   //Now things are a little more tricky as we are going to try and 
   //work out which Version we have and use the appropriate pointer


   strncpy(rootFileName,outName,FILENAME_MAX);
   //   std::cout << sizeof(FullSurfHkStruct_t) << std::endl;
   std::ifstream PosFile(inName);

   int numBytes=0;
   char fileName[180];
   int error=0;
   int counter=0;
   int firstTime=1;
   int version=VER_SURF_HK;
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
	 std::cout << fileName << "\t" << theSurfHk.threshold[0][0] << std::endl;
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
	 if(gHdr.code != PACKET_SURF_HK) {
	    std::cerr << "not a FullSurfHkStruct_t\n";
	    exit(0);
	 }
	 if(gHdr.verId != VER_SURF_HK) {
	    std::cout << "Old version of FullSurfHkStruct_t -- " << (int)gHdr.verId
		 << std::endl;
	    switch(gHdr.verId) {
	    case 14:
	       if(gHdr.numBytes==sizeof(FullSurfHkStructVer14_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=14;
	       }
	       break;
	    case 13:
	       if(gHdr.numBytes==sizeof(FullSurfHkStructVer13_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=13;
	       }
	       break;
	    case 12:
	    case 11:
	    case 10:
	       if(gHdr.numBytes==sizeof(FullSurfHkStructVer12_t)) {
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
	      << VER_SURF_HK << ")\n";
      }

      for(int i=0;i<1000;i++) {	
	 if(version==VER_SURF_HK) {
	    numBytes=gzread(infile,&theSurfHk,sizeof(FullSurfHkStruct_t));
	    if(numBytes!=sizeof(FullSurfHkStruct_t)) {
	       error=1;
	       break;
	    }
	 }
	 else {
	    int numBytesExpected=sizeof(FullSurfHkStruct_t);
	    switch(version) {
	    case 14:
	       numBytesExpected=sizeof(FullSurfHkStructVer14_t);
	       numBytes=gzread(infile,&theSurfHkVer14,numBytesExpected);
	       break;
	    case 13:
	       numBytesExpected=sizeof(FullSurfHkStructVer13_t);
	       numBytes=gzread(infile,&theSurfHkVer13,numBytesExpected);
	       break;
	    case 12:
	       numBytesExpected=sizeof(FullSurfHkStructVer12_t);
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
	    //	    std::cout << theSurfHk.gHdr.code << "\t" << theSurfHk.unixTime << std::endl;
	 processSurfHk(version);
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
	surfhkTree = new TTree("surfHkTree","Tree of Anita Scalers And Thresholds");
	surfhkTree->Branch("surf","SurfHk",&surfHkPtr);
   
	doneInit=1;
    }
    //    std::cout << theSurfHk.threshold[0][0] << std::endl;
    if(surfHkPtr) delete surfHkPtr;
    if(version==VER_SURF_HK) {
       surfHkPtr = new SurfHk(runNumber,theSurfHk.unixTime,&theSurfHk);
    }
    else {
       switch(version) {
       case 14:
	  surfHkPtr = new SurfHk(runNumber,theSurfHkVer14.unixTime,&theSurfHkVer14);
	  break;
       case 13:
	  surfHkPtr = new SurfHk(runNumber,theSurfHkVer13.unixTime,&theSurfHkVer13);
	  break;
       case 12:
	  surfHkPtr = new SurfHk(runNumber,theSurfHkVer12.unixTime,&theSurfHkVer12);
	  break;
       default:
	  std::cout << "And shouldn't ver get here\n";
	  exit(0);
       }
    }
			   
    surfhkTree->Fill();                
}



