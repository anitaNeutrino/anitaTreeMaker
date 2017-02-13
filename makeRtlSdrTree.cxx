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
void makeRtlSdrTree(int nin, char **inNames, char *outName);

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

  //check if there is more than one ROOT file, if so, then the second shoul be a calibraiton file 


  int nroot =0; 
  int iout = -1; 
  int ical = -1; 

  for (int i = 1; i < argc; i++)
  {
    if (strstr(argv[i],".root"))
    {
      nroot++; 

      if (nroot == 1) iout = i; 
      if (nroot == 2) ical = i; 


    }
  }

  if (ical > 2)
  {
    TFile *f = new TFile(argv[ical]); 

    for (int i = 0; i < NUM_RTLSDR; i++) 
    {
      calibration[i] = (TGraph*) f->Get(TString::Format("RTL%d", i+1)); 
    }
  }

  makeRtlSdrTree(iout-1, argv+1,argv[iout]);
  return 0;
}


void makeRtlSdrTree(int nin, char **inNames, char *outName) {

  if (nin == 0) return; 

  strncpy(rootFileName,outName,FILENAME_MAX);


   for (int i = 0; i < nin; i++) 
   {
   std::ifstream PosFile(inNames[i]);

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
