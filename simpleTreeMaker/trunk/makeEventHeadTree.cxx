#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <zlib.h>
#include <libgen.h>
#include <errno.h>
   

#include "TTree.h"
#include "TFile.h"
#include "TSystem.h"

#define HACK_FOR_ROOT

#include "RawAnitaHeader.h"
#include "simpleStructs.h"

using namespace std;

void processHeader(int version);
void makeRunHeadTree(char *inName, char *outName);

AnitaEventHeader_t theHeader;
AnitaEventHeaderVer13_t theHeader13;
AnitaEventHeaderVer12_t theHeader12;
AnitaEventHeaderVer11_t theHeader11;
AnitaEventHeaderVer10_t theHeader10;

RawAnitaHeader *theHead;
TFile *theFile;
TTree *headTree;
UInt_t realTime;
Int_t runNumber;
char rootFileName[FILENAME_MAX];

int main(int argc, char **argv) {
  if(argc<3) {
    std::cout << "Usage: " << basename(argv[0]) << " <file list> <outfile>" << std::endl;
    return -1;
  }
  makeRunHeadTree(argv[1],argv[2]);
  return 0;
}


void makeRunHeadTree(char *inName, char *outName) {
//    std::cout << sizeof(AnitaEventHeader_t) << std::endl;
   
//   char inName[FILENAME_MAX];
//   sprintf(inName,"run%dFileList.txt",run);
//   sprintf(rootFileName,"/data/anita/ANITA-novtest/root/run%d/headFile%d.root",run,run);
  strncpy(rootFileName,outName,FILENAME_MAX);
  ifstream SillyFile(inName);

    int numBytes=0;
    char fileName[180];
    int error=0;
//    int headerNumber=1;
    int counter=0;


    int firstTime=1;
    GenericHeader_t gHdr;
    int version=VER_EVENT_HEADER;

    std::cout << sizeof(AnitaEventHeader_t) << std::endl;
    //    return;
    while(SillyFile >> fileName) {
      //if(counter%100==0) 
	std::cout << fileName << std::endl;
	counter++;
	
	const char *subDir = gSystem->DirName(fileName);
	const char *subSubDir = gSystem->DirName(subDir);
	const char *eventDir = gSystem->DirName(subSubDir);
	const char *runDir = gSystem->DirName(eventDir);
	const char *justRun = gSystem->BaseName(runDir);
//	std::cout << justRun << std::endl;
	sscanf(justRun,"run%d",&runNumber);

	gzFile infile = gzopen (fileName, "rb");   

	if(firstTime) {
	   //Need to work out which version this is
	   numBytes=gzread(infile,&gHdr,sizeof(GenericHeader_t));
	   if(numBytes!=sizeof(GenericHeader_t)) {
	      std::cerr << "Error reading GenericHeader_t to determine version\n";
	      exit(0);
	   }
	   gzrewind(infile);
	   if(gHdr.code != PACKET_HD) {
	      std::cerr << "not an AnitaEventHeader_t\n";
	      exit(0);
	   }

	 if(gHdr.verId != VER_EVENT_HEADER) {
	    std::cout << "Old version of AnitaEventHeader_t -- " << (int)gHdr.verId
		 << std::endl;
	    switch(gHdr.verId) {
	    case 13:
	       if(gHdr.numBytes==sizeof(AnitaEventHeaderVer13_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=13;
	       }
	       break;
	    case 12:
	       if(gHdr.numBytes==sizeof(AnitaEventHeaderVer12_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=12;
	       }
	       break;
	    case 11:
	       if(gHdr.numBytes==sizeof(AnitaEventHeaderVer11_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=11;
	       }
	       break;	       
	    case 10:
	       if(gHdr.numBytes==sizeof(AnitaEventHeaderVer10_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=10;
	       }
	       break;
	    default:
	       std::cerr << "This version is not currently supported someone needs, to update me\n";
	       exit(0);
	    }
	 }	 
	   std::cout << "Got version:\t" << int(gHdr.verId) << " current " 
		     << VER_EVENT_HEADER << "\n";
	   firstTime=0;
	}


 
	//	cout << infile << " " << fileName << endl;
	for(int i=0;i<100;i++) {	
	  //	  std::cout << i << std::endl;
	   int numBytesExpected=sizeof(AnitaEventHeader_t);
	   if(version==VER_EVENT_HEADER) {
	      numBytes=gzread(infile,&theHeader,sizeof(AnitaEventHeader_t));
	   }
	   else {
	    switch(version) {
	    case 13:
	       numBytesExpected=sizeof(AnitaEventHeaderVer13_t);
	       numBytes=gzread(infile,&theHeader13,numBytesExpected);
	       break;
	    case 12:
	       numBytesExpected=sizeof(AnitaEventHeaderVer12_t);
	       numBytes=gzread(infile,&theHeader12,numBytesExpected);
	       break;
	    case 11:
	       numBytesExpected=sizeof(AnitaEventHeaderVer11_t);
	       numBytes=gzread(infile,&theHeader11,numBytesExpected);
	       break;
	    case 10:
	       numBytesExpected=sizeof(AnitaEventHeaderVer10_t);
	       numBytes=gzread(infile,&theHeader10,numBytesExpected);
	       break;
	    default:
	       std::cerr << "Shouldn't ever get here\n";
	       exit(0);
	    }
	      
	   }
	    //	    cout << i << "\t" << numBytes << endl;
	    if(numBytes==-1) {
	      int errorNum=0;
	      cout << gzerror(infile,&errorNum) << "\t" << errorNum << endl;
	    }
	    if(numBytes!=numBytesExpected) {	      
	      if(numBytes!=0) {
		error=1;
		break;
	      }
	      else break;
	    }
	    processHeader(version);
	}
	gzclose(infile);
//	if(error) break;
    }
    theFile->Write();
}

#define C3PO_AVG 20

void processHeader(int version) {
  static int doneInit=0;
  if(!doneInit) {
    //    char name[128];
    //    char def[128];
    theFile = new TFile(rootFileName,"RECREATE");
    
    headTree = new TTree("headTree","Tree of Anita Event Headers");
    headTree->Branch("header","RawAnitaHeader",&theHead);    
    doneInit=1;
  }
  if(theHead) delete theHead;
  
  

  //This is wrong, but good enough for telemetry
  UInt_t trigTime=theHeader.turfio.trigTime;
  UInt_t triggerTime=theHeader.unixTime;
  UInt_t triggerTimeNs=1e9*(trigTime/250e6);
  Int_t goodTimeFlag=1;
    
   
  if(version==VER_EVENT_HEADER) {
    theHead = new RawAnitaHeader(&theHeader,runNumber,realTime,triggerTime,triggerTimeNs,goodTimeFlag);
  }

  headTree->Fill();                
}
