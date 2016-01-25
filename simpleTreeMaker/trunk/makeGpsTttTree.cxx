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

using namespace std;

void makeGpsTttTree(char *tttInName);
void setupOutTree();


GpsSubTime_t rawSubTime;


TFile *theFile;
char rootFileName[FILENAME_MAX];
TTree *tttTree;

int doneInit=0;

int runNumber;
UInt_t realTime;



int main(int argc, char **argv) {
  if(argc<3) {
    std::cout << "Usage: " << basename(argv[0]) << " <ttt infile> outfile>" << std::endl;
    return -1;
  }
  strncpy(rootFileName,argv[2],FILENAME_MAX);
  makeGpsTttTree(argv[1]);
  return 0;
}



void makeGpsTttTree(char *tttInName)
{
  setupOutTree();
  {
    std::cout << "Starting TTT" << endl;
    ifstream GpsFile(tttInName);      
    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
    static int firstTtt=1;
    while(GpsFile >> fileName) {
      const char *subDir = gSystem->DirName(fileName);
      const char *subSubDir = gSystem->DirName(subDir);
      const char *typeDir = gSystem->DirName(subSubDir);
      const char *unitDir= gSystem->DirName(typeDir);
      const char *gpsDir = gSystem->DirName(unitDir);
      const char *houseDir = gSystem->DirName(gpsDir);
      const char *runDir = gSystem->DirName(houseDir);
      const char *justRun = gSystem->BaseName(runDir);
      //	std::cout << justRun << std::endl;
      sscanf(justRun,"run%d",&runNumber);
      
      if(counter%100==0)
	std::cout << fileName << "\t" << runNumber << std::endl;
      counter++;
      gzFile infile = gzopen (fileName, "rb");    
      for(int i=0;i<1000;i++) {	
	numBytes=gzread(infile,&rawSubTime,sizeof(GpsSubTime_t));
	if(numBytes!=sizeof(GpsSubTime_t)) {
	  error=1;
	  break;
	}
	tttTree->Fill();
      }
      gzclose(infile);
      //	if(error) break;
    }
    tttTree->AutoSave();
  }

  theFile->Close();
}


void setupOutTree()
{
  theFile = new TFile(rootFileName,"RECREATE");
  tttTree = new TTree("tttTree","Tree of GPS Sub Time Files");
  tttTree->Branch("unixTimeGps",&rawSubTime.unixTime,"unixTimeGps/i");
  tttTree->Branch("subTimeGps",&rawSubTime.subTime,"subTimeGps/i");
  tttTree->Branch("fromAdu5",&rawSubTime.fromAdu5,"fromAdu5/I");
}
