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

#include "TurfRate.h"
#include "simpleStructs.h"


void processTurfRate(int version);
void makeTurfRateTree(char *inName, char *outName);

//Global Variable
TurfRateStruct_t theTurfRate;
TurfRateStructVer34_t theTurfRateVer34;
TurfRateStructVer16_t theTurfRateVer16;
TurfRateStructVer15_t theTurfRateVer15;
TurfRateStructVer14_t theTurfRateVer14;
TurfRateStructVer13_t theTurfRateVer13;
TurfRateStructVer12_t theTurfRateVer12;
TurfRateStructVer11_t theTurfRateVer11;
TFile *theFile;
TTree *turfRateTree=0;
TurfRate *theTurfRateClass;
char rootFileName[FILENAME_MAX];
int doneInit=0;
int runNumber;
UInt_t realTime;


int main(int argc, char **argv) {
   if(argc<3) {
      std::cout << "Usage: " << basename(argv[0]) << " <file list> <outfile>" << std::endl;
      return -1;
   }
   makeTurfRateTree(argv[1],argv[2]);
   return 0;
}


void makeTurfRateTree(char *inName, char *outName) {
   strncpy(rootFileName,outName,FILENAME_MAX);
   std::ifstream PosFile(inName);
    
   int numBytes=0;
   char fileName[180];
   int error=0;
   int counter=0;

   int firstTime=1;
   GenericHeader_t gHdr;
   int version=VER_TURF_RATE;

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
	 if(gHdr.code != PACKET_TURF_RATE) {
	    std::cerr << "not a TurfRateStruct_t\n";
	    exit(0);
	 }

	 if(gHdr.verId != VER_TURF_RATE) {
	    std::cout << "Old version of TurfRateStruct_t -- " << (int)gHdr.verId
		      << std::endl;
	    switch(gHdr.verId) {
	    case 34:
	       if(gHdr.numBytes==sizeof(TurfRateStructVer34_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=34;
	       }
	       break;
	    case 16:
	       if(gHdr.numBytes==sizeof(TurfRateStructVer16_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=16;
	       }
	       break;
	    case 15:
	       if(gHdr.numBytes==sizeof(TurfRateStructVer15_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=15;
	       }
	       break;
	    case 14:
	       if(gHdr.numBytes==sizeof(TurfRateStructVer14_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=14;
	       }
	       break;
	    case 13:
	       if(gHdr.numBytes==sizeof(TurfRateStructVer13_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=13;
	       }
	       break;
	    case 12:
	       if(gHdr.numBytes==sizeof(TurfRateStructVer12_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=12;
	       }
	       break;
	    case 11:
	       if(gHdr.numBytes==sizeof(TurfRateStructVer11_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=11;
	       }
	       break;
	    default:
	       std::cerr << "This version is not currently supported someone needs, to update me\n";
	       exit(0);
	    }
	 }	    
	 std::cout << "Got version:\t" << version << " current " 
		   << VER_TURF_RATE << "\n";
	 firstTime=0;
      }

      for(int i=0;i<1000;i++) {	
	 if(version==VER_TURF_RATE) {
	    numBytes=gzread(infile,&theTurfRate,sizeof(TurfRateStruct_t));
	    if(numBytes!=sizeof(TurfRateStruct_t)) {
	       error=1;
	       break;
	    }
	 }
	 else {
	    int numBytesExpected=sizeof(TurfRateStruct_t);
	    switch(version) {
	    case 34:
	       numBytesExpected=sizeof(TurfRateStructVer34_t);
	       numBytes=gzread(infile,&theTurfRateVer34,numBytesExpected);
	       break;
	    case 16:
	       numBytesExpected=sizeof(TurfRateStructVer16_t);
	       numBytes=gzread(infile,&theTurfRateVer16,numBytesExpected);
	       break;
	    case 15:
	       numBytesExpected=sizeof(TurfRateStructVer15_t);
	       numBytes=gzread(infile,&theTurfRateVer15,numBytesExpected);
	       break;
	    case 14:
	       numBytesExpected=sizeof(TurfRateStructVer14_t);
	       numBytes=gzread(infile,&theTurfRateVer14,numBytesExpected);
	       break;
	    case 13:
	       numBytesExpected=sizeof(TurfRateStructVer13_t);
	       numBytes=gzread(infile,&theTurfRateVer13,numBytesExpected);
	       break;
	    case 12:
	       numBytesExpected=sizeof(TurfRateStructVer12_t);
	       numBytes=gzread(infile,&theTurfRateVer12,numBytesExpected);
	       break;
	    case 11:
	       numBytesExpected=sizeof(TurfRateStructVer11_t);
	       numBytes=gzread(infile,&theTurfRateVer11,numBytesExpected);
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
	 processTurfRate(version);
      }
      gzclose(infile);
      //	if(error) break;
   }
    
   turfRateTree->Write();
   theFile->Close();
   doneInit=0;
}

void processTurfRate(int version) {
  static Int_t lastPpsNum=-1;
   if(!doneInit) {
      theFile = new TFile(rootFileName,"RECREATE");

      turfRateTree = new TTree("turfRateTree","Tree of Anita TURF Trigger Rates");
      turfRateTree->Branch("turf","TurfRate",&theTurfRateClass);       	

      doneInit=1;
      lastPpsNum=-1;
   }
   if(theTurfRate.ppsNum==lastPpsNum) return;
   lastPpsNum=theTurfRate.ppsNum;
   //    std::cout << theTurfRate.unixTime << "\t" 
   ///	      << theTurfRate.ppsNum << "\t"
   //	      << theTurfRate.gHdr.code << "\n";
   if(version==VER_TURF_RATE) {
      theTurfRateClass = new TurfRate(runNumber,
				      theTurfRate.unixTime,
				      &theTurfRate);
   }
   else {
      switch(version) {	 
       case 34:
	  theTurfRateClass = new TurfRate(runNumber,theTurfRateVer34.unixTime,&theTurfRateVer34);
	  break;
       case 16:
	  theTurfRateClass = new TurfRate(runNumber,theTurfRateVer16.unixTime,&theTurfRateVer16);
	  break;
       case 15:
	  theTurfRateClass = new TurfRate(runNumber,theTurfRateVer15.unixTime,&theTurfRateVer15);
	  break;
       case 14:
	  theTurfRateClass = new TurfRate(runNumber,theTurfRateVer14.unixTime,&theTurfRateVer14);
	  break;
       case 13:
	  theTurfRateClass = new TurfRate(runNumber,theTurfRateVer13.unixTime,&theTurfRateVer13);
	  break;
       case 12:
	  theTurfRateClass = new TurfRate(runNumber,theTurfRateVer12.unixTime,&theTurfRateVer12);
	  break;
       case 11:
	  theTurfRateClass = new TurfRate(runNumber,theTurfRateVer11.unixTime,&theTurfRateVer11);
	  break;
       default:
	  std::cout << "And shouldn't ever get here\n";
	  exit(0);
      }      
   }
			       
   turfRateTree->Fill();  
   delete theTurfRateClass;
}




