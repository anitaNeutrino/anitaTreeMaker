#include <cstdio>
#include <fstream>
#include <iostream>
#include <zlib.h>
#include <libgen.h>     
#include <utime.h>    
#include <map>  

using namespace std;

#include "TTree.h"
#include "TFile.h"
#include "TSystem.h"

#define HACK_FOR_ROOT

#include "simpleStructs.h"
#include "RawAnitaEvent.h"  

void processBody();
void makeBodyTree(char *inputName, char *outDir);
int checkFileTimeAgainstRootTime(char *fileName);
void fillEventNumberMap(char *rootName);
int eventInMap(UInt_t eventNumber);


std::map<UInt_t,UInt_t> eventNumberMap;

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

void fillEventNumberMap(char *rootName) {

  theFile = new TFile(rootName,"OLD");
  eventTree = new TTree("eventTree","Tree of Anita Events");
  eventTree->Branch("run",&runNumber,"run/I");
  eventTree->Branch("event","RawAnitaEvent",&theEvent);

  for(int i=0;i<eventTree->GetEntries();i++) {
    eventTree->GetEntry(i);
    eventNumberMap.insert(std::pair<UInt_t,UInt_t>(theEvent.eventNumber,theEvent.eventNumber));
  }
  delete theFile;
  theFile=0;
  eventTree=0;
}


int checkFileTimeAgainstRootTime(char *fileName) {
  static int gotRunTime=0;
  static struct stat bufRootFile;  
  if(!gotRunTime) {    
      char dirName[FILENAME_MAX];
      char rootName[FILENAME_MAX];
      sprintf(dirName,"%s/run%d",outDirName,runNumber);
      sprintf(rootName,"%s/eventFile%d.root",dirName,runNumber);
      int retVal2=stat(rootName,&bufRootFile);  
      if(retVal2==0) { 
	//File exists
	std::cout << rootName << "\t" << bufRootFile.st_mtime << "\n";
	fillEventNumberMap(rootName);
      }
      else {
	bufRootFile.st_mtime=0;
      }
      gotRunTime=1;
  }

  
  struct stat bufRawFile;
  int retVal2=stat(rootName,&bufRawFile);  
  if(retVal2==0) { 
    if(bufRawFile.st_mtime>bufRootFile.st_mtime)
      return 1;
  }
  return 0;
  
}

int eventInMap(UInt_t eventNumber) {
  std::map::iterator it=eventNumberMap.find(eventNumber);
  if(it==eventNumberMap.end()) {
    eventNumberMap.insert(std::pair<UInt_t,UInt_t>(eventNumber,eventNumber));
    return 0;
  }
  return 1;

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
    
    if(checkFileTimeAgainstRootTime(fileName)) {
      std::cout << "File too old\n";
      continue;
    }

    gzFile infile = gzopen (fileName, "rb"); 
    for(int i=0;i<100;i++) {

      numBytes=gzread(infile,&theBody,sizeof(PedSubbedEventBody_t));
      if(numBytes!=sizeof(PedSubbedEventBody_t)) {
	if(numBytes>0) {
	  cerr << "Read problem: " << i << "\t" <<  numBytes << " of " << sizeof(PedSubbedEventBody_t) << " Last Event -- " << lastEventNumber << endl;
	  cout << fileName << endl;
	}
	error=1;
	break;
      }

      //RJN hack for now
      if(runNumber>=10000 && runNumber<=10057) {
	if(counter>1 && i==1) {
	  if(theBody.eventNumber!=lastEventNumber+1) {
	    gzrewind(infile);
	    numBytes=gzread(infile,&theBody,sizeof(PedSubbedEventBody_t));
	    gzread(infile,&theBody,616);
	    numBytes=gzread(infile,&theBody,sizeof(PedSubbedEventBody_t));
	  }
	}
      }
      //      cout << "Event: " << i << "\t" << theBody.eventNumber << endl;
      if(eventInMap(theBody.eventNumber)) continue;
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
