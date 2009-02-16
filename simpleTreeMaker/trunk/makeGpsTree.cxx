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
#include "Adu5Pat.h"
#include "Adu5Sat.h"
#include "Adu5Vtg.h"
#include "G12Pos.h"
#include "G12Sat.h"
#include "GpsGga.h"

using namespace std;

void makeGpsTree(char *adu5PatInName, char *adu5SatInName, char *asu5VtgInName,
		 char *g12PosInName, char *g12SatInName, char *gpsGgaInName);
void setupOutTree();

//Global Variable
Adu5Pat *adu5PatPtr=0;
Adu5Sat *adu5SatPtr=0;
Adu5Vtg *adu5VtgPtr=0;
G12Pos *g12PosPtr=0;
G12Sat *g12SatPtr=0;
GpsGga *gpsGgaPtr=0;

GpsAdu5PatStruct_t rawAdu5Pat;
GpsAdu5SatStruct_t rawAdu5Sat;
GpsAdu5VtgStruct_t rawAdu5Vtg;
GpsG12PosStruct_t rawG12Pos;
GpsG12SatStruct_t rawG12Sat;
GpsGgaStruct_t rawGpsGga;


TFile *theFile;
char rootFileName[FILENAME_MAX];
TTree *adu5PatTree;
TTree *adu5SatTree;
TTree *adu5VtgTree;
TTree *adu5bPatTree;
TTree *adu5bSatTree;
TTree *adu5bVtgTree;
TTree *g12PosTree;
TTree *g12SatTree;
TTree *g12GgaTree;
TTree *adu5GgaTree;
TTree *adu5bGgaTree;

int doneInit=0;

int runNumber;
UInt_t realTime;



int main(int argc, char **argv) {
  if(argc<8) {
    std::cout << "Usage: " << basename(argv[0]) << " <adu5 pat infile> <adu5 sat infile> <adu5 vtg infile> <g12 pos infile> <g12 sat infile> <gps gga infile> <outfile>" << std::endl;
    return -1;
  }
  strncpy(rootFileName,argv[7],FILENAME_MAX);
  makeGpsTree(argv[1],argv[2],argv[3],argv[4],argv[5],argv[6]);
  return 0;
}



void makeGpsTree(char *adu5PatInName, char *adu5SatInName, char *adu5VtgInName,
		 char *g12PosInName, char *g12SatInName, char *gpsGgaInName)
{
  setupOutTree();
  {
    std::cout << "Starting ADU5 PAT" << endl;
    ifstream GpsFile(adu5PatInName);      
    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
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
	std::cout << fileName << std::endl;
      counter++;
      gzFile infile = gzopen (fileName, "rb");    
      for(int i=0;i<1000;i++) {	
	numBytes=gzread(infile,&rawAdu5Pat,sizeof(GpsAdu5PatStruct_t));
	if(numBytes!=sizeof(GpsAdu5PatStruct_t)) {
	  error=1;
	  break;
	}
	if(adu5PatPtr) delete adu5PatPtr;
	adu5PatPtr = new Adu5Pat(runNumber,rawAdu5Pat.unixTime,
				 &rawAdu5Pat);
	if(rawAdu5Pat.gHdr.code&PACKET_FROM_ADU5B)
	  adu5bPatTree->Fill();
	else 
	  adu5PatTree->Fill();
      }
      gzclose(infile);
      //	if(error) break;
    }
    adu5PatTree->AutoSave();
    adu5bPatTree->AutoSave();
  }

  {
    cout << "Starting ADU5 SAT" << endl;
    ifstream GpsFile(adu5SatInName);      
    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
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
	std::cout << fileName << std::endl;
      counter++;
      gzFile infile = gzopen (fileName, "rb");    
      for(int i=0;i<1000;i++) {	
	numBytes=gzread(infile,&rawAdu5Sat,sizeof(GpsAdu5SatStruct_t));
	if(numBytes!=sizeof(GpsAdu5SatStruct_t)) {
	  error=1;
	  break;
	}
	if(adu5SatPtr) delete adu5SatPtr;
	adu5SatPtr = new Adu5Sat(runNumber,rawAdu5Sat.unixTime,
				 &rawAdu5Sat);
	if(rawAdu5Sat.gHdr.code&PACKET_FROM_ADU5B)
	  adu5bSatTree->Fill();
	else 
	  adu5SatTree->Fill();
      }
      gzclose(infile);
      //	if(error) break;
    }
    adu5SatTree->AutoSave();
    adu5bSatTree->AutoSave();
  }
  {
    cout << "Starting ADU5 VTG" << endl;
    ifstream GpsFile(adu5VtgInName);      
    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
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
	std::cout << fileName << std::endl;
      counter++;
      gzFile infile = gzopen (fileName, "rb");    
      for(int i=0;i<1000;i++) {	
	numBytes=gzread(infile,&rawAdu5Vtg,sizeof(GpsAdu5VtgStruct_t));
	if(numBytes!=sizeof(GpsAdu5VtgStruct_t)) {
	  error=1;
	  break;
	}
	if(adu5VtgPtr) delete adu5VtgPtr;
	adu5VtgPtr = new Adu5Vtg(runNumber,rawAdu5Vtg.unixTime,
				 &rawAdu5Vtg);
	if(rawAdu5Vtg.gHdr.code&PACKET_FROM_ADU5B)
	  adu5bVtgTree->Fill();
	else 
	  adu5VtgTree->Fill();
      }
      gzclose(infile);
      //	if(error) break;
    }
    adu5VtgTree->AutoSave();
    adu5bVtgTree->AutoSave();
  }
  {
    ifstream GpsFile(g12PosInName);      
    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
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
	std::cout << fileName << std::endl;
      counter++;
      gzFile infile = gzopen (fileName, "rb");    
      for(int i=0;i<1000;i++) {	
	numBytes=gzread(infile,&rawG12Pos,sizeof(GpsG12PosStruct_t));
	if(numBytes!=sizeof(GpsG12PosStruct_t)) {
	  error=1;
	  break;
	}
	if(g12PosPtr) delete g12PosPtr;
	g12PosPtr = new G12Pos(runNumber,rawG12Pos.unixTime,
			       &rawG12Pos);
	g12PosTree->Fill();
      }
      gzclose(infile);
      //	if(error) break;
    }
    g12PosTree->AutoSave();
  }
  {
    cout << "Starting G12 SAT" << endl;
    ifstream GpsFile(g12SatInName);      
    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
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
	std::cout << fileName << std::endl;
      counter++;
      gzFile infile = gzopen (fileName, "rb");    
      for(int i=0;i<1000;i++) {	
	numBytes=gzread(infile,&rawG12Sat,sizeof(GpsG12SatStruct_t));
	if(numBytes!=sizeof(GpsG12SatStruct_t)) {
	  error=1;
	  break;
	}
	if(g12SatPtr) delete g12SatPtr;
	g12SatPtr = new G12Sat(runNumber,rawG12Sat.unixTime,
			       &rawG12Sat);
	g12SatTree->Fill();
      }
      gzclose(infile);
      //	if(error) break;
    }
    g12SatTree->AutoSave();
  }
  {
    cout << "Starting GPS GGA" << endl;
    ifstream GpsFile(gpsGgaInName);      
    int numBytes=0;
    char fileName[180];
    int error=0;
    int counter=0;
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
	std::cout << fileName << std::endl;
      counter++;
      gzFile infile = gzopen (fileName, "rb");    
      for(int i=0;i<1000;i++) {	
	numBytes=gzread(infile,&rawGpsGga,sizeof(GpsGgaStruct_t));
	if(numBytes!=sizeof(GpsGgaStruct_t)) {
	  error=1;
	  break;
	}
	if(gpsGgaPtr) delete gpsGgaPtr;
	gpsGgaPtr = new GpsGga(runNumber,rawGpsGga.unixTime,
			       &rawGpsGga);	
	if(rawGpsGga.gHdr.code&PACKET_FROM_G12)
	  g12GgaTree->Fill();
	else if(rawGpsGga.gHdr.code&PACKET_FROM_ADU5B)
	  adu5bGgaTree->Fill();
	else
	  adu5GgaTree->Fill();
      }
      gzclose(infile);
      //	if(error) break;
    }
    g12GgaTree->AutoSave();
    adu5GgaTree->AutoSave();
    adu5bGgaTree->AutoSave();
  }
  theFile->Close();
}


void setupOutTree()
{
  theFile = new TFile(rootFileName,"RECREATE");
  adu5PatTree = new TTree("adu5PatTree","Tree of ADU5 Position and Attitude Info");
  adu5PatTree->Branch("pat","Adu5Pat",&adu5PatPtr);

  adu5SatTree = new TTree("adu5SatTree","Tree of ADU5 Satellite Info");
  adu5SatTree->Branch("sat","Adu5Sat",&adu5SatPtr);

  adu5VtgTree = new TTree("adu5VtgTree","Tree of ADU5 Velocity and Course Info");
  adu5VtgTree->Branch("vtg","Adu5Vtg",&adu5VtgPtr);

  adu5bPatTree = new TTree("adu5bPatTree","Tree of ADU5B Position and Attitude Info");
  adu5bPatTree->Branch("pat","Adu5Pat",&adu5PatPtr);

  adu5bSatTree = new TTree("adu5bSatTree","Tree of ADU5B Satellite Info");
  adu5bSatTree->Branch("sat","Adu5Sat",&adu5SatPtr);

  adu5bVtgTree = new TTree("adu5bVtgTree","Tree of ADU5B Velocity and Course Info");
  adu5bVtgTree->Branch("vtg","Adu5Vtg",&adu5VtgPtr);

  g12PosTree = new TTree("g12PosTree","Tree of G12 Position Info");
  g12PosTree->Branch("pos","G12Pos",&g12PosPtr);
  g12SatTree = new TTree("g12SatTree","Tree of G12 Satellite Info");
  g12SatTree->Branch("sat","G12Sat",&g12SatPtr);
  g12GgaTree = new TTree("g12GgaTree","Tree of GPS Geoid Info");
  g12GgaTree->Branch("gga","GpsGga",&gpsGgaPtr);
  
  adu5GgaTree = new TTree("adu5GgaTree","Tree of GPS Geoid Info");
  adu5GgaTree->Branch("gga","GpsGga",&gpsGgaPtr);
  
  adu5bGgaTree = new TTree("adu5bGgaTree","Tree of GPS Geoid Info");
  adu5bGgaTree->Branch("gga","GpsGga",&gpsGgaPtr);
}
