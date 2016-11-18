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

#include "simpleStructs.h"
#include "AcqdStart.h"
#include "GpsdStart.h"
#include "LogWatchdStart.h"
#include "RunStart.h"
#include "CommandEcho.h"


void makeAuxTree(char *acqdStartInName, char *gpsdStartInName, 
		 char *logWatchdStartInName, char *commandEchoInName);
void setupOutTree();

//Global Variable
AcqdStart *acqdStartPtr=0;
GpsdStart *gpsdStartPtr=0;
LogWatchdStart *logWatchdStartPtr=0;
//RunStart *runStartPtr=0;
CommandEcho *cmdEchoPtr=0;

AcqdStartStruct_t rawAcqdStart;
GpsdStartStruct_t rawGpsdStart;
LogWatchdStart_t rawLogWatchdStart;
//RunStart_t *rawRunStartPtr;
CommandEcho_t rawCmdEcho;


TFile *theFile;
char rootFileName[FILENAME_MAX];
TTree *acqdStartTree;
TTree *gpsdStartTree;
TTree *logWatchdStartTree;
TTree *runStartTree;
TTree *cmdEchoTree;

int runNumber;
UInt_t realTime;



int main(int argc, char **argv) {
  if(argc<6) {
    std::cout << "Usage: " << basename(argv[0]) << " <acqd start infile> <gpsd start infile> <logWatchd start infile>  <command echo infile> <outfile>" << std::endl;
    return -1;
  }
  strncpy(rootFileName,argv[5],FILENAME_MAX);
  makeAuxTree(argv[1],argv[2],argv[3],argv[4]);
  return 0;
}



void makeAuxTree(char *acqdStartInName, char *gpsdStartInName, 
		 char *logWatchdStartInName, char *commandEchoInName)
{
  std::cerr << "makeAuxTree\n" << acqdStartInName << "\t"
	    << gpsdStartInName << "\n"
	    << logWatchdStartInName << "\n"
	    << commandEchoInName << "\n";
  setupOutTree();
  {
    std::ifstream PosFile(acqdStartInName);      
    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
    while(PosFile >> fileName) {
      const char *startDir = gSystem->DirName(fileName);      	
      const char *runDir = gSystem->DirName(startDir);
      const char *justRun = gSystem->BaseName(runDir);
      //	std::cout << justRun << std::endl;
      sscanf(justRun,"run%d",&runNumber);
      
      if(counter%100==0)
	std::cout << fileName << std::endl;
      counter++;
      gzFile infile = gzopen (fileName, "rb");    
      for(int i=0;i<1000;i++) {	
	numBytes=gzread(infile,&rawAcqdStart,sizeof(AcqdStartStruct_t));
	if(numBytes!=sizeof(AcqdStartStruct_t)) {
	  error=1;
	  break;
	}
	if(acqdStartPtr) delete acqdStartPtr;
	acqdStartPtr = new AcqdStart(runNumber,rawAcqdStart.unixTime,
				     &rawAcqdStart);
	acqdStartTree->Fill();
      }
      gzclose(infile);
      //	if(error) break;
    }
    acqdStartTree->AutoSave();
  }
  {
    std::ifstream PosFile(gpsdStartInName);      
    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
    while(PosFile >> fileName) {
      const char *startDir = gSystem->DirName(fileName);      	
      const char *runDir = gSystem->DirName(startDir);
      const char *justRun = gSystem->BaseName(runDir);
      //	std::cout << justRun << std::endl;
      sscanf(justRun,"run%d",&runNumber);
      
      if(counter%100==0)
	std::cout << fileName << std::endl;
      counter++;
      gzFile infile = gzopen (fileName, "rb");    
      for(int i=0;i<1000;i++) {	
	numBytes=gzread(infile,&rawGpsdStart,sizeof(GpsdStartStruct_t));
	if(numBytes!=sizeof(GpsdStartStruct_t)) {
	  error=1;
	  break;
	}
	if(gpsdStartPtr) delete gpsdStartPtr;
	gpsdStartPtr = new GpsdStart(runNumber,rawGpsdStart.unixTime,
				     &rawGpsdStart);
	gpsdStartTree->Fill();
      }
      gzclose(infile);
      //	if(error) break;
    }
    gpsdStartTree->AutoSave();
  }
  {
    std::ifstream PosFile(logWatchdStartInName);      
    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
    while(PosFile >> fileName) {
      const char *startDir = gSystem->DirName(fileName);      	
      const char *runDir = gSystem->DirName(startDir);
      const char *justRun = gSystem->BaseName(runDir);
      //	std::cout << justRun << std::endl;
      sscanf(justRun,"run%d",&runNumber);
      
      if(counter%100==0)
	std::cout << fileName << std::endl;
      counter++;
      gzFile infile = gzopen (fileName, "rb");    
      for(int i=0;i<1000;i++) {	
	numBytes=gzread(infile,&rawLogWatchdStart,sizeof(LogWatchdStart_t));
	if(numBytes!=sizeof(LogWatchdStart_t)) {
	  error=1;
	  break;
	}
	if(logWatchdStartPtr) delete logWatchdStartPtr;
	logWatchdStartPtr = new LogWatchdStart(runNumber,rawLogWatchdStart.unixTime,
				     &rawLogWatchdStart);
	logWatchdStartTree->Fill();
      }
      gzclose(infile);
      //	if(error) break;
    }
    logWatchdStartTree->AutoSave();
  }
  {
    std::ifstream PosFile(commandEchoInName);      
    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
    while(PosFile >> fileName) {
      const char *subDir = gSystem->DirName(fileName);
      const char *subSubDir = gSystem->DirName(subDir);
      const char *cmdDir= gSystem->DirName(subSubDir);
      const char *houseDir = gSystem->DirName(cmdDir);
      const char *runDir = gSystem->DirName(houseDir);
      const char *justRun = gSystem->BaseName(runDir);
      //	std::cout << justRun << std::endl;
      sscanf(justRun,"run%d",&runNumber);
      
      if(counter%100==0)
	std::cout << fileName << std::endl;
      counter++;
      gzFile infile = gzopen (fileName, "rb");    
      for(int i=0;i<1000;i++) {	
	numBytes=gzread(infile,&rawCmdEcho,sizeof(CommandEcho_t));
	if(numBytes!=sizeof(CommandEcho_t)) {
	  error=1;
	  break;
	}
	if(cmdEchoPtr) delete cmdEchoPtr;
	cmdEchoPtr = new CommandEcho(runNumber,rawCmdEcho.unixTime,
				     &rawCmdEcho);
	cmdEchoTree->Fill();
      }
      gzclose(infile);
      //	if(error) break;
    }
    cmdEchoTree->AutoSave();
  }
  theFile->Close();
}


void setupOutTree()
{
  theFile = new TFile(rootFileName,"RECREATE");
  acqdStartTree = new TTree("acqdStartTree","Tree of Acqd Start Structs");
  acqdStartTree->Branch("acqd","AcqdStart",&acqdStartPtr);
  gpsdStartTree = new TTree("gpsdStartTree","Tree of Gpsd Start Structs");
  gpsdStartTree->Branch("gpsd","GpsdStart",&gpsdStartPtr);
  logWatchdStartTree = new TTree("logWatchdStartTree","Tree of LogWatchd Start Structs");
  logWatchdStartTree->Branch("log","LogWatchdStart",&logWatchdStartPtr);
  cmdEchoTree = new TTree("cmdEchoTree","Tree of Command Echo Structs");
  cmdEchoTree->Branch("cmd","CommandEcho",&cmdEchoPtr);
}
