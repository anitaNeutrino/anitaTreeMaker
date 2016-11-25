#include <stdio.h>
#include "TuffNotchStatus.h" 
#include "TFile.h"
#include "TTree.h" 
#include <fstream>
#include <iostream>
#include "TSystem.h"
#include <zlib.h>

TuffNotchStatus *theTuffPtr;
TuffNotchStatus_t theTuff;
TFile *theFile;
TTree *tuffTree;
char rootFileName[FILENAME_MAX];
int doneInit=0;
int runNumber;
UInt_t realTime;

void processTuffNotch() {

  if(!doneInit) {
	theFile = new TFile(rootFileName,"RECREATE");

    tuffTree = new TTree("tuffTree","Tree of Anita tuff status");
    tuffTree->Branch("tuff","TuffNotchStatus",&theTuffPtr);
    doneInit=1;
   }

  if(theTuffPtr) delete theTuffPtr;
    theTuffPtr = new TuffNotchStatus(runNumber,&theTuff);
    tuffTree->Fill();
    std::cout << theTuff.gHdr.code << "\t" << theTuff.gHdr.packetNumber << "\t"
	      << (int) theTuff.gHdr.feByte << "\t" << (int) theTuff.gHdr.verId << "\t"
	      << (int) theTuff.gHdr.checksum << std::endl;
    // PacketCode_t code;
    // unsigned int packetNumber; ///<Especially for Ped
    // unsigned short numBytes;
    // unsigned char feByte;
    // unsigned char verId;
    // unsigned int checksum;

}




void makeTuffNotchTree(char *inName, char *outName) {
   std::ifstream PosFile(inName);

    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
    while(PosFile >> fileName) {
    const char *subDir = gSystem->DirName(fileName);
    const char *subSubDir = gSystem->DirName(subDir);
    const char *tuffDir= gSystem->DirName(subSubDir);
    const char *houseDir = gSystem->DirName(tuffDir);
    const char *runDir = gSystem->DirName(houseDir);
    const char *justRun = gSystem->BaseName(runDir);
  //	std::cout << justRun << std::endl;
    sscanf(justRun,"run%d",&runNumber);


	if(counter%100==0)
	    std::cout << fileName << std::endl;
	counter++;
	gzFile infile = gzopen (fileName, "rb");
	for(int i=0;i<1000;i++) {
	    numBytes=gzread(infile,&theTuff,sizeof(TuffNotchStatus_t));
	    if(numBytes!=sizeof(TuffNotchStatus_t)) {
		error=1;
		break;
	    }
	    processTuffNotch();
	}
	gzclose(infile);
    }

    tuffTree->AutoSave();
    theFile->Close();
    doneInit=0;
}

int main(int nargs, char ** args) 
{
  if (nargs < 3) 
  {
    fprintf(stderr, "Usage: %s <file list> <outfile>\n", args[0]); 
    return 1; 
  }



  makeTuffNotchTree(args[1],args[2]); 


  return 0; 

}


