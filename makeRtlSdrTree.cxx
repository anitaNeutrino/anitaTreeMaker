#include <cstdio>
#include <fstream>
#include <iostream>
#include <zlib.h>
#include <libgen.h>


#include "TTree.h"
#include "TFile.h"
#include "TSystem.h"
#include "TTreeIndex.h"

#define HACK_FOR_ROOT

#include "simpleStructs.h"
#include "RTLSpectrum.h"
#include "AnitaConventions.h" 

static TGraph * calibration[NUM_RTLSDR] = {0}; 

void processRtlSdr();
void makeRtlSdrTree(char *inName, char *outName);

//Global Variable
RTLSpectrum *theRtlSdrPtr;
RtlSdrPowerSpectraStruct_t theRtlSdr;
TFile *theFile;
TTree *rtlTree;
char rootFileName[FILENAME_MAX];
int doneInit=0;
int runNumber;
UInt_t realTime;

int main(int argc, char **argv) {
  if(argc<3) {
    std::cout << "Usage: " << basename(argv[0]) << " <file list> <outfile> [calibration_file]" << std::endl;
    return -1;
  }

  if (argc > 2)
  {
    TFile *f = new TFile(argv[3]); 

    for (int i = 0; i < NUM_RTLSDR; i++) 
    {
      calibration[i] = (TGraph*) f->Get(TString::Format("RTL%d", i+1)); 
    }
  }

  makeRtlSdrTree(argv[1],argv[2]);
  return 0;
}


void makeRtlSdrTree(char *inName, char *outName) {
   strncpy(rootFileName,outName,FILENAME_MAX);

   std::ifstream PosFile(inName);

    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
    while(PosFile >> fileName) {
       const char *subDir = gSystem->DirName(fileName);
       const char *subSubDir = gSystem->DirName(subDir);
       const char *rtlDir= gSystem->DirName(subSubDir);
       const char *houseDir = gSystem->DirName(rtlDir);
       const char *runDir = gSystem->DirName(houseDir);
       const char *justRun = gSystem->BaseName(runDir);
//       std::cout << justRun << std::endl;
       sscanf(justRun,"run%d",&runNumber);


       if(counter%100==0)
           std::cout << fileName << std::endl;
       counter++;
       gzFile infile = gzopen (fileName, "rb");
       for(int i=0;i<1000;i++) {
           numBytes=gzread(infile,&theRtlSdr,sizeof(RtlSdrPowerSpectraStruct_t));
           if(numBytes!=sizeof(RtlSdrPowerSpectraStruct_t)) {
              error=1;
              break;
           }
           processRtlSdr();
       }
       gzclose(infile);
    }

    rtlTree->AutoSave();
    theFile->Close();
    doneInit=0;
}

void processRtlSdr() {
    if(!doneInit) {
       theFile = new TFile(rootFileName,"RECREATE");
       rtlTree = new TTree("rtlTree","Tree of Anita RT Spectra Stuff");
       rtlTree->Branch("rtl","RTLSpectrum",&theRtlSdrPtr);
       doneInit=1;
    }
    if(theRtlSdrPtr) delete theRtlSdrPtr;
    theRtlSdrPtr = new RTLSpectrum(&theRtlSdr,calibration[theRtlSdr.rtlNum-1]);
    rtlTree->Fill();

}
