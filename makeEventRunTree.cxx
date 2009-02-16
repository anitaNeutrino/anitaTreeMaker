#include <cstdio>
#include <fstream>
#include <iostream>
#include <zlib.h>
#include <libgen.h>     
 
using namespace std;

#include "TTree.h"
#include "TFile.h"
#include "TSystem.h"

#define HACK_FOR_ROOT

#include "simpleStructs.h"
#include "RawAnitaEvent.h"  

void processBody();
void makeBodyTree(char *inputName, char *outDir);

PedSubbedEventBody_t theBody;
TFile *theFile;
TTree *eventTree;
RawAnitaEvent *theEvent=0;
char outDirName[FILENAME_MAX];
UInt_t realTime;
Int_t runNumber;
Int_t lastRunNumber;


int main(int argc, char **argv) {
  if(argc<3) {
    std::cout << "Usage: " << basename(argv[0]) << " <file list> <out dir>" << std::endl;
    return -1;
  }
  makeBodyTree(argv[1],argv[2]);
  return 0;
}
  

void makeBodyTree(char *inputName, char *outDir) {
  cout << inputName << "\t" << outDir << endl;
  strncpy(outDirName,outDir,FILENAME_MAX);
  theEvent = new RawAnitaEvent();
  //    cout << sizeof(PedSubbedEventBody_t) << endl;
  ifstream SillyFile(inputName);

  int numBytes=0;
  char fileName[180];
  int error=0;
  //    int bodyNumber=1;
  int counter=0;
  int lastEventNumber=0;
  while(SillyFile >> fileName) {
    if(counter%100==0) 
      cout << fileName << endl;
    counter++;
    
    const char *subDir = gSystem->DirName(fileName);
    const char *subSubDir = gSystem->DirName(subDir);
    const char *eventDir = gSystem->DirName(subSubDir);
    const char *runDir = gSystem->DirName(eventDir);
    const char *justRun = gSystem->BaseName(runDir);
    //	cout << justRun << endl;
    sscanf(justRun,"run%d",&runNumber);
    
    gzFile infile = gzopen (fileName, "rb");    
    for(int i=0;i<100;i++) {	
      //      cout << i << endl;
      numBytes=gzread(infile,&theBody,sizeof(PedSubbedEventBody_t));
      if(numBytes!=sizeof(PedSubbedEventBody_t)) {
	if(numBytes>0) {
	  cerr << "Read problem: " << i << "\t" <<  numBytes << " of " << sizeof(PedSubbedEventBody_t) << " Last Event -- " << lastEventNumber << endl;
	  cout << fileName << endl;
	}
	error=1;
	break;
      }
      //      cout << "Event: " << theBody.eventNumber << endl;
      processBody();
      lastEventNumber=theBody.eventNumber;
    }
    gzclose(infile);
    //	if(error) break;
  }
  eventTree->AutoSave();
  //    theFile->Close();
}


void processBody() {
  //  cout << "processBody:\t" << theBody.eventNumber << endl;
  static int doneInit=0;
  
  if(!doneInit) {
    char dirName[FILENAME_MAX];
    char fileName[FILENAME_MAX];
    sprintf(dirName,"%s/run%d",outDirName,runNumber);
    gSystem->mkdir(dirName,kTRUE);
    sprintf(fileName,"%s/eventFile%d.root",dirName,runNumber);
    cout << "Creating File: " << fileName << endl;
    theFile = new TFile(fileName,"RECREATE");
    eventTree = new TTree("eventTree","Tree of Anita Events");
    eventTree->Branch("run",&runNumber,"run/I");
    eventTree->Branch("event","RawAnitaEvent",&theEvent);
    
    doneInit=1;
  }  
  //  cout << "Here: "  << theBody.eventNumber << endl;
  if(theEvent) delete theEvent;
  theEvent = new RawAnitaEvent(&theBody);
  eventTree->Fill();  
  lastRunNumber=runNumber;
  //  delete theEvent;
}
