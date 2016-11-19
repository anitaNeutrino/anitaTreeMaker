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
int eventInMap(UInt_t eventNumber);


std::map<UInt_t,UInt_t> eventNumberMap;

AnitaEventHeader_t theHeader;
AnitaEventHeaderVer40_t theHeader40;
AnitaEventHeaderVer33_t theHeader33;
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




int eventInMap(UInt_t eventNumber) {
  if(eventNumber==0) return 1; //Bug arggh fixed around run 18
  std::map<UInt_t,UInt_t>::iterator it=eventNumberMap.find(eventNumber);
  if(it==eventNumberMap.end()) {
    eventNumberMap.insert(std::pair<UInt_t,UInt_t>(eventNumber,eventNumber));
    return 0;
  }
  return 1;

}

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
  std::ifstream SillyFile(inName);

    int numBytes=0;
    char fileName[180];
    //    int error=0;
//    int headerNumber=1;
    int counter=0;


    int firstTime=1;
    GenericHeader_t gHdr;
    int version=VER_EVENT_HEADER;

    std::cout << sizeof(AnitaEventHeader_t) << std::endl;
    //    return;
    while(SillyFile >> fileName) {
      if(counter%100==0) 
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
	    case 40:
	      std::cout << gHdr.numBytes << "\t" << sizeof(AnitaEventHeader_t) << "\n";
	       if(gHdr.numBytes==sizeof(AnitaEventHeaderVer40_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=40;
	       }
	       break;
	    case 33:
	       if(gHdr.numBytes==sizeof(AnitaEventHeaderVer33_t)) {
		  std::cout << "Size matches will proceed\n";
		  version=33;
	       }
	       break;
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
	  int thisEventNumber=0;
	  //	  std::cout << i << std::endl;
	   int numBytesExpected=sizeof(AnitaEventHeader_t);
	   if(version==VER_EVENT_HEADER) {
	      numBytes=gzread(infile,&theHeader,sizeof(AnitaEventHeader_t));
	      thisEventNumber=theHeader.eventNumber;
	   }
	   else {
	    switch(version) {
	    case 40:
	       numBytesExpected=sizeof(AnitaEventHeaderVer40_t);
	       numBytes=gzread(infile,&theHeader40,numBytesExpected);
	       thisEventNumber=theHeader40.eventNumber;
	       //	       std::cout << "Got header40: " << thisEventNumber << "\t" << (int)theHeader40.gHdr.code << "\t" << (int) theHeader40.gHdr.verId << "\n";
	       break;
	    case 33:
	       numBytesExpected=sizeof(AnitaEventHeaderVer33_t);
	       numBytes=gzread(infile,&theHeader33,numBytesExpected);
	       thisEventNumber=theHeader33.eventNumber;
	       //	       std::cout << "Got header33: " << thisEventNumber << "\t" << (int)theHeader33.gHdr.code << "\t" << (int) theHeader33.gHdr.verId << "\n";
	       break;
	    case 13:
	       numBytesExpected=sizeof(AnitaEventHeaderVer13_t);
	       numBytes=gzread(infile,&theHeader13,numBytesExpected);
	       thisEventNumber=theHeader13.eventNumber;
	       break;
	    case 12:
	       numBytesExpected=sizeof(AnitaEventHeaderVer12_t);
	       numBytes=gzread(infile,&theHeader12,numBytesExpected);
	       thisEventNumber=theHeader12.eventNumber;
	       break;
	    case 11:
	       numBytesExpected=sizeof(AnitaEventHeaderVer11_t);
	       numBytes=gzread(infile,&theHeader11,numBytesExpected);
	       thisEventNumber=theHeader11.eventNumber;
	       break;
	    case 10:
	       numBytesExpected=sizeof(AnitaEventHeaderVer10_t);
	       numBytes=gzread(infile,&theHeader10,numBytesExpected);
	       thisEventNumber=theHeader10.eventNumber;
	       break;
	    default:
	       std::cerr << "Shouldn't ever get here\n";
	       exit(0);
	    }
	      
	   }
	   //	   std::cout << "Version: " << version << "\t" << numBytes << "\t" << numBytesExpected << "\n";
	    //	    cout << i << "\t" << numBytes << endl;
	    if(numBytes==-1) {
	      int errorNum=0;
	      cout << gzerror(infile,&errorNum) << "\t" << errorNum << endl;
	    }
	    if(numBytes!=numBytesExpected) {	      
	      if(numBytes!=0) {
		//		error=1;
		break;
	      }
	      else break;
	    } 
	    if(eventInMap(thisEventNumber)) {
	      //	std::cout << "Got event: " << theBody.eventNumber << "\n";
	      continue;
	    }
	    processHeader(version);
	}
	gzclose(infile);
//	if(error) break;
    }
    theFile->Write();
}

void makeHeadTree() {
//    std::cout << sizeof(AnitaEventHeader_t) << std::endl;
    std::ifstream SillyFile("sillyFileOfFilenames.txt");
    sprintf(rootFileName,"/unix/anita1/tempTrees/bufferHeadFile.root");
    int numBytes=0;
    char fileName[180];
    //    int error=0;
//    int headerNumber=1;
    int counter=0;
    while(SillyFile >> fileName) {
	if(counter%100==0) 
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
	for(int i=0;i<100;i++) {	
//	    std::cout << i << std::endl;
	    numBytes=gzread(infile,&theHeader,sizeof(AnitaEventHeader_t));
	    cout << i << "\t" << numBytes << endl;
	    if(numBytes!=sizeof(AnitaEventHeader_t)) {
	      //		error=1;
		break;
	    }
	    processHeader(VER_EVENT_HEADER);
	}
	gzclose(infile);
//	if(error) break;
    }
    headTree->AutoSave();;
    theFile->Close();
}
#define C3PO_AVG 20

void processHeader(int version) {
  //  std::cout << "processHeader( " << version << " )\n";
  static UInt_t c3poNumArray[C3PO_AVG]={0};
  static UInt_t ppsNumArray[C3PO_AVG]={0};
  static UInt_t c3poCounter=0;
  static int doneInit=0;
  static UInt_t lastTriggerTime=0;
  static UInt_t ppsOffset=0;
  static UInt_t lastPps=0;
  UInt_t ppsNotReset=0;
  if(!doneInit) {
    //    char name[128];
    //    char def[128];
    std::cout << "Creating " << rootFileName << "\n";
    theFile = new TFile(rootFileName,"RECREATE");
    
    headTree = new TTree("headTree","Tree of Anita Event Headers");
    headTree->Branch("header","RawAnitaHeader",&theHead);
    
    
    doneInit=1;
  }
  if(theHead) delete theHead;
  
  UInt_t ppsNum=theHeader.turfio.ppsNum;
  UInt_t unixTime=theHeader.unixTime;
  UInt_t trigTime=theHeader.turfio.trigTime;
  UInt_t c3poNum=theHeader.turfio.c3poNum;
  UInt_t rawppsNum=theHeader.turfio.ppsNum;
  UInt_t rawtrigTime=theHeader.turfio.trigTime;
  UInt_t rawc3poNum=theHeader.turfio.c3poNum;
  
  if(version != VER_EVENT_HEADER) {
    switch(version) {
    case 40:
	  ppsNum=theHeader40.turfio.ppsNum;
	  unixTime=theHeader40.unixTime;
	  trigTime=theHeader40.turfio.trigTime;
	  c3poNum=theHeader40.turfio.c3poNum;
	  rawppsNum=theHeader40.turfio.ppsNum;
	  rawtrigTime=theHeader40.turfio.trigTime;
	  rawc3poNum=theHeader40.turfio.c3poNum;
	  break;
    case 33:
	  ppsNum=theHeader33.turfio.ppsNum;
	  unixTime=theHeader33.unixTime;
	  trigTime=theHeader33.turfio.trigTime;
	  c3poNum=theHeader33.turfio.c3poNum;
	  break;
    case 13:
	  ppsNum=theHeader13.turfio.ppsNum;
	  unixTime=theHeader13.unixTime;
	  trigTime=theHeader13.turfio.trigTime;
	  c3poNum=theHeader13.turfio.c3poNum;
	  break;
       case 12:
	  ppsNum=theHeader12.turfio.ppsNum;
	  unixTime=theHeader12.unixTime;
	  trigTime=theHeader12.turfio.trigTime;
	  c3poNum=theHeader12.turfio.c3poNum;
	  break;
       case 11:
	  ppsNum=theHeader11.turfio.ppsNum;
	  unixTime=theHeader11.unixTime;
	  trigTime=theHeader11.turfio.trigTime;
	  c3poNum=theHeader11.turfio.c3poNum;
	  break;
       case 10:
	  ppsNum=theHeader10.turfio.ppsNum;
	  unixTime=theHeader10.unixTime;
	  trigTime=theHeader10.turfio.trigTime;
	  c3poNum=theHeader10.turfio.c3poNum;
	  break;
       default:
	  std::cerr << "Shouldn't be here\n";
	  break;
       }
    }

  ppsNotReset=0;
    if(c3poCounter==0) {
      c3poNumArray[c3poCounter]=c3poNum;
      ppsNumArray[c3poCounter]=ppsNum;
      c3poCounter++;
    }
    else {
      Int_t lastIndex=(c3poCounter-1)%C3PO_AVG;
      Int_t thisIndex=(c3poCounter%C3PO_AVG);
      //      std::cout << thisIndex << "\t" << lastIndex << "\t" << ppsNum << "\t" << ppsNumArray[lastIndex] << "\n";
      if(ppsNum!=ppsNumArray[lastIndex] && (c3poNum>249.996e6) && (c3poNum<249.998e6) ) {
	c3poNumArray[thisIndex]=c3poNum;
	ppsNumArray[thisIndex]=ppsNum;
	c3poCounter++;
      }
      UInt_t numToAvg=C3PO_AVG;
      if(c3poCounter<numToAvg) numToAvg=c3poCounter;
      Long64_t c3poTemp=0;
      for(UInt_t i=0;i<numToAvg;i++) {
	c3poTemp+=c3poNumArray[i];
      }
      c3poTemp/=numToAvg;
      c3poNum=c3poTemp;
      if(version==VER_EVENT_HEADER) 
	theHeader.turfio.c3poNum=c3poNum; //<Here we are doing naughty things
      else if(version==33)
	theHeader33.turfio.c3poNum=c3poNum;
      else if(version==40)
	theHeader40.turfio.c3poNum=c3poNum;
      
      //      std::cout << c3poTemp << "\t" << numToAvg << "\t" << c3poNum << "\n";
    }


    if(ppsOffset==0 || ppsNum<lastPps) {
      //Need to reset the ppsOffset;  
      ppsOffset=unixTime-ppsNum;
      while(lastTriggerTime>ppsNum+ppsOffset)
	ppsOffset++;
    }
    //    std::cout << c3poNum << std::endl;
    realTime=unixTime;
    UInt_t triggerTimeNs=trigTime;
    Int_t goodTimeFlag=0;
    
      //Good c3poNum;
    if(trigTime<=c3poNum) {
      //Good trigTime
      goodTimeFlag=1;
    }
    else {
      trigTime=trigTime-c3poNum;
      ppsNotReset=1;
    }
    triggerTimeNs=(UInt_t)(1e9*(Double_t(trigTime)/Double_t(c3poNum)));


    UInt_t triggerTime=ppsNum+ppsOffset+ppsNotReset;
    lastTriggerTime=triggerTime;
    lastPps=ppsNum;
    //    std::cout << "Trigger Time " << triggerTime << " " << ppsNum << " " << ppsOffset << " " << ppsNotReset << std::endl;


    
    if(version==VER_EVENT_HEADER) {
       theHead = new RawAnitaHeader(&theHeader,runNumber,realTime,triggerTime,triggerTimeNs,goodTimeFlag);
       theHead->trigTime    = trigTime;
       theHead->rawtrigTime = rawtrigTime;
       theHead->rawppsNum   = rawppsNum + ppsNotReset;
       theHead->rawc3poNum  = rawc3poNum;
    }
    else {
       switch(version) {
       case 40:
	 //	 std::cout << "Trying with version 40\n";
	 theHead = new RawAnitaHeader((AnitaEventHeaderVer40_t*)&theHeader40,runNumber,realTime,triggerTime,triggerTimeNs,goodTimeFlag);
	 theHead->trigTime    = trigTime;
	 theHead->rawtrigTime = rawtrigTime;
	 theHead->rawppsNum   = rawppsNum + ppsNotReset;
	 theHead->rawc3poNum  = rawc3poNum;
	 break;
       case 33:
	 //	 std::cout << "Trying with version 33\n";
	 theHead = new RawAnitaHeader((AnitaEventHeaderVer33_t*)&theHeader33,runNumber,realTime,triggerTime,triggerTimeNs,goodTimeFlag);
	 break;
       case 13:
	  theHead = new RawAnitaHeader(&theHeader13,runNumber,realTime,triggerTime,triggerTimeNs,goodTimeFlag);
	  break;
       case 12:
	  theHead = new RawAnitaHeader(&theHeader12,runNumber,realTime,triggerTime,triggerTimeNs,goodTimeFlag);
	  break;
       case 11:
	  theHead = new RawAnitaHeader(&theHeader11,runNumber,realTime,triggerTime,triggerTimeNs,goodTimeFlag);
	  break;
       case 10:
	  theHead = new RawAnitaHeader(&theHeader10,runNumber,realTime,triggerTime,triggerTimeNs,goodTimeFlag);
	  break;
       default:
	  std::cerr << "Shouldn't be here!!!\n";
	  break;
       }
    }
   // std::cout << theHeader.eventNumber << std::endl;

    headTree->Fill();                
}
