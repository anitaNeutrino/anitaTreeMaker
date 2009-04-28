#include <iostream>
#include <vector>
#include <math.h>
#include <stdio.h>

#include "TFile.h"
#include "TTree.h"

#include "Adu5Pat.h"
#include "RawAnitaHeader.h"
#include "simpleStructs.h"

using namespace std;

void makeAdu5PatTree(int doingRun);

int main(int argc, char**argv) {
  if(argc<2) {
    cerr << "Usage:\n\t" << argv[0] << "\t<run no.>" << endl;
    return -1;
  }
  int doingRun=atoi(argv[1]);
  makeAdu5PatTree(doingRun);
  return 0;
}


//void smoothing(TBranch *branch, vector<double> *sBranch, float value, int sIndex){

void smoothing(vector<double> inBranch, int sIndex, vector<double> &sBranch){

  //  vector<double> container;
  double sValue = 0.;
  int index =0;
  
  //smooth the values in the branch; make sure to NOT smooth the first and 
  //last few points in the data set (sIndex/2), because those indicies will 
  //be out of range of our vector
  int min = floor(sIndex/2);
  //int max = inBranch->GetEntries()-min;
  int max = inBranch.size()-min-1;
  //  for(int i = min; i<max; i++){
  for(int i = 0; i<inBranch.size()-1; i++){
    sValue = 0.; 
    if(i>=min && i<max){
      for (int j = 0; j<=sIndex; j++){
	index = i-min+j;      
	sValue += inBranch.at(index)*1./(double(sIndex)+1.) ;
      }
    } else sValue = inBranch.at(index);
    sBranch.push_back(sValue);
    
  }
}


void makeAdu5PatTree(int doingRun) {


  //First up open header file
  char headName[FILENAME_MAX];
  //ANITA II has a different format than ANITA I, and the root files are already made
  sprintf(headName,"/raid2/grashorn/anita/data/anita2/flightData/run%d/headFile%d.root",doingRun,doingRun); 
  TFile *fHead= new TFile(headName,"OLD");
  TTree *headTree = (TTree*) fHead->Get("headTree");
  
  RawAnitaHeader *theHeader=0;
  headTree->SetBranchAddress("header",&theHeader);

  //ANITA II has a different format than ANITA I, and the 
  //gps files are already made
  char gpsName[FILENAME_MAX]; 
  sprintf(gpsName,"/raid2/grashorn/anita/data/anita2/flightData/run%d/gpsFile%d.root",doingRun,doingRun); 
  TFile *fGps = new TFile(gpsName,"OLD");//"/raid2/grashorn/anita/data/anita2/fixedTimeFiles/gpsFile.root");    
  
  TTree *adu5PatTree = (TTree*)fGps->Get("adu5PatTree");

  //Declaration of leaves types

   UInt_t          realTime=0;
   UInt_t          readTime=0;
   UInt_t          payloadTime=0;
   UInt_t          payloadTimeUs=0;
   UInt_t          timeOfDay=0;
   Float_t         latitude=0.;
   Float_t         longitude=0.;
   Float_t         altitude=0.;
   Float_t         heading=0.;
   Float_t         pitch=0.;
   Float_t         roll=0.;
   Float_t         mrms=0.;
   Float_t         brms=0.;
   UInt_t          attFlag=0;

   Adu5Pat *adu5PatPtr = 0;
   
   adu5PatTree->SetBranchAddress("pat",&adu5PatPtr);
   adu5PatTree->BuildIndex("realTime","payloadTimeUs");
   
   
   //Declaration of leaf types
   UInt_t          frealTime = 0;
   UInt_t          freadTime=0;
   UInt_t          fpayloadTime=0;
   UInt_t          ftimeOfDay=0;
   Float_t         flatitude=0;
   Float_t         flongitude=0;
   Float_t         faltitude=0;
   Float_t         fheading=0;
   Float_t         fpitch=0;
   Float_t         froll=0;
   Float_t         fmrms=0;
   Float_t         fbrms=0;
   UInt_t          fattFlag=0;

   UInt_t          srealTime=0;
   UInt_t          stimeOfDay=0;
   Float_t         slatitude=0;
   Float_t         slongitude=0;
   Float_t         saltitude=0;
   Float_t         sheading=0;
   Float_t         spitch=0;
   Float_t         sroll=0;
   Float_t         smrms=0;
   Float_t         sbrms=0;
   UInt_t          sattFlag=0;



   char outName[FILENAME_MAX];
   //sprintf(outName,"/raid2/grashorn/anita/data/anita2/flightData/run%d/gpsEvent%d.root",doingRun,doingRun);  
   sprintf(outName,"/raid2/grashorn/anita/runTreeMaker/gpsEvent%d.root",doingRun);  
   TFile *newFp= new TFile(outName,"RECREATE"); 
   
   TTree *newTree= new TTree("adu5PatTree","Tree of Interpolated ADU5 Positions and Attitude");;
   Adu5Pat *interpAdu5Pat = new Adu5Pat();
   newTree->Branch("pat","Adu5Pat",&interpAdu5Pat);
   int bestEntryLast=0;
   
   //added 02.13.09 EW Grashorn, so that we're compatible w/ new version of Adu5Pat.cxx
   Int_t interpolationFlag=-1;
   Long64_t hkEntries = adu5PatTree->GetEntries();
   Long64_t nentries = headTree->GetEntries();

   vector<double> pitchVec;
   vector<double> rollVec;
   vector<double> sPitchVec;
   vector<double> sRollVec;
   vector<double> ssRollVec;
   vector<double> ssPitchVec;

   //fill pitch and roll vectors
   for(int i = 0; i < hkEntries; i++){
     adu5PatTree->GetEntry(i);
     pitchVec.push_back(adu5PatPtr->pitch);
     rollVec.push_back(adu5PatPtr->roll);
   }

   //smoothing(vector<double> inBranch, int sIndex, vector<double> &sBranch){
   smoothing(rollVec, 5, sRollVec);
   smoothing(sRollVec, 5, ssRollVec);
   smoothing(pitchVec, 5, sPitchVec);
   smoothing(sPitchVec, 5, ssPitchVec);

   Int_t finterpolationFlag=0;
   Int_t sinterpolationFlag=0;

   Long64_t nbytes = 0;
   Long64_t starEvery=nentries/20;
   if(starEvery==0) starEvery=1;
   for (Long64_t i=0; i<nentries;i++) {
     if(i%starEvery==0) cerr << "*";

     nbytes += headTree->GetEntry(i);
      Long64_t bestEntry=adu5PatTree->GetEntryNumberWithBestIndex(theHeader->triggerTime,theHeader->triggerTimeNs/1000);
     
      if(bestEntry<0) {
	cerr << "Something bad happened: " << i << bestEntry <<endl;
	timeOfDay=-9999;
	latitude=-9999; 
	longitude=-9999;
	altitude=-9999; 
	heading=-9999;  
	pitch=-9999;    
	roll=-9999;     
	mrms=-9999;     
	brms=-9999;     
	attFlag=-9999;    

	//continue;
	//exit(0);
      }
      else {
	if(bestEntry>=(hkEntries-1) && bestEntry>0) {
	  bestEntry--;
	}
	adu5PatTree->GetEntry(bestEntry);
	
	while(adu5PatPtr->realTime<theHeader->triggerTime){
	  bestEntry++;
	  adu5PatTree->GetEntry(bestEntry);
	}
	while(adu5PatPtr->realTime>theHeader->triggerTime){
	  bestEntry--;
	  adu5PatTree->GetEntry(bestEntry);
	} 
	
	//Copy to temporary variables
	frealTime=adu5PatPtr->realTime;
	fpayloadTime=adu5PatPtr->payloadTime;
	freadTime=adu5PatPtr->payloadTime;
	ftimeOfDay=adu5PatPtr->timeOfDay;
	flatitude=adu5PatPtr->latitude;
	flongitude=adu5PatPtr->longitude;
	faltitude=adu5PatPtr->altitude;
	fheading=adu5PatPtr->heading;
	fpitch=ssPitchVec.at(bestEntry);
	//fpitch=adu5PatPtr->pitch;
	froll=ssRollVec.at(bestEntry);
	//froll=adu5PatPtr->roll;
	fmrms=adu5PatPtr->mrms;
	fbrms=adu5PatPtr->brms;
	fattFlag=adu5PatPtr->attFlag;

	bestEntry++;
	adu5PatTree->GetEntry(bestEntry);
	
	//Copy to temporary variables
	srealTime=adu5PatPtr->realTime;
	stimeOfDay=adu5PatPtr->timeOfDay;
	slatitude=adu5PatPtr->latitude;
	slongitude=adu5PatPtr->longitude;
	saltitude=adu5PatPtr->altitude;
	sheading=adu5PatPtr->heading;
	//spitch=adu5PatPtr->pitch;
	spitch=ssPitchVec.at(bestEntry);//fpitch;
	//sroll=adu5PatPtr->roll;
	sroll=ssRollVec.at(bestEntry);//froll;
	smrms=adu5PatPtr->mrms;
	sbrms=adu5PatPtr->brms;
	sattFlag=adu5PatPtr->attFlag;
	

	realTime=theHeader->triggerTime;
	payloadTimeUs=theHeader->triggerTimeNs/1000;
	payloadTime=(fpayloadTime-frealTime)+realTime;
	readTime=(freadTime-freadTime)+realTime;
	//finterpolationFlag=(Int_t)TMath::Abs(double(frealTime)-double(theHeader->triggerTime));
	finterpolationFlag=(Int_t)TMath::Abs(double(frealTime-1)-double(theHeader->triggerTime));
	sinterpolationFlag=(Int_t)TMath::Abs(double(srealTime)-double(theHeader->triggerTime));
	
	interpolationFlag=finterpolationFlag;
	if(sinterpolationFlag<interpolationFlag) interpolationFlag=sinterpolationFlag;

	if(finterpolationFlag*60<sinterpolationFlag) {
	  //Just use first numbers;
	  timeOfDay=ftimeOfDay;
	  latitude=flatitude;
	  longitude=flongitude;
	  altitude=faltitude;
	  heading=fheading;
	  pitch=fpitch;
	  roll=froll;
	  mrms=fmrms;
	  brms=fbrms;
	  attFlag=fattFlag;
	  

	}
	else if(sinterpolationFlag*60<finterpolationFlag) {
	  //Just use second numbers
	  timeOfDay=stimeOfDay;
	  latitude=slatitude;
	  longitude=slongitude;
	  altitude=saltitude;
	  heading=sheading;
	  pitch=spitch;
	  roll=sroll;
	  mrms=smrms;
	  brms=sbrms;
	  attFlag=sattFlag;
	  cout<<"Ever? "<<theHeader->eventNumber<<endl;	
	  
	}
	else {
	  //Need to interpolate
	  Double_t t=Double_t(theHeader->triggerTime)+1e-9*Double_t(theHeader->triggerTimeNs);
	  Double_t t1=Double_t(frealTime);
	  Double_t t2=Double_t(srealTime);
	  Double_t timeFactor=(t-t1)/(t2-t1);
	  
	  if(fattFlag==sattFlag) {
	    timeOfDay=(UInt_t)(ftimeOfDay + timeFactor*(stimeOfDay-ftimeOfDay));
	  
	    latitude=flatitude + timeFactor*(slatitude-flatitude);
	    longitude=flongitude + timeFactor*(slongitude-flongitude);
	    altitude=faltitude + timeFactor*(saltitude-faltitude);
// 	    if(TMath::Abs(360+sheading-fheading)<TMath::Abs(sheading-fheading)) {
// 	      sheading+=360;
// 	    }
// 	    if(TMath::Abs(sheading-(fheading+360))<TMath::Abs(sheading-fheading)) {
// 	      fheading+=360;
// 	    }
	    
// 	    heading=fheading + timeFactor*(sheading-fheading);
// 	    if(heading>=360) {
// 	      heading-=360;
// 	    } 


	    heading=fheading + timeFactor*(sheading-fheading);
	    pitch=fpitch + timeFactor*(spitch-fpitch);
	    roll=froll + timeFactor*(sroll-froll);
	    mrms=fmrms + timeFactor*(smrms-fmrms);
	    brms=fbrms + timeFactor*(sbrms-fbrms);
	    attFlag=fattFlag;


	  }
	  else if(fattFlag==0) {
	    //Just use first numbers
	    timeOfDay=ftimeOfDay;
	    latitude=flatitude;
	    longitude=flongitude;
	    altitude=faltitude;
	    heading=fheading;
	    pitch=fpitch;
	    roll=froll;
	    mrms=fmrms;
	    brms=fbrms;
	    attFlag=fattFlag;
	  }
	  else {
	    //Just use second numbers
	    timeOfDay=stimeOfDay;
	    latitude=slatitude;
	    longitude=slongitude;
	    altitude=saltitude;
	    heading=sheading;
	    pitch=spitch;
	    roll=sroll;
	    mrms=smrms;
	    brms=sbrms;
	    attFlag=sattFlag;
	  }
	    
	  
	}
      }

      if(
	 theHeader->eventNumber==107690600//||theHeader->eventNumber==1198819
	 //||
	 //theHeader->eventNumber>1076806&&theHeader->eventNumber<1076905
	 //finterpolationFlag!=1||sinterpolationFlag!=1
	 ){
	cout<<theHeader->eventNumber<<" "<<theHeader->run
	    <<" "<<endl
	  //<<" "<<interpolationFlag<<" "<<finterpolationFlag<<" "<<sinterpolationFlag<<endl
	  //<<" "<<timeOfDay-ftimeOfDay<<" "<<stimeOfDay-timeOfDay<<endl
	  //  <<" "<<ftimeOfDay<<" "<<timeOfDay<<" "<<stimeOfDay<<endl
	    <<" "<<theHeader->triggerTime<<" "<<1e-9*Double_t(theHeader->triggerTimeNs)<<endl
	    <<" "<<frealTime<<" "<<ftimeOfDay<<" "<<timeOfDay<<" "<<endl
	  //  <<" "<<payloadTime<<" "<<realTime<<endl
	  //  <<" "<<ftimeOfDay<<" "<<timeOfDay<<" "<<stimeOfDay<<endl
	  //  <<" "<<fpitch<<" "<<pitch<<" "<<spitch<<endl
	  //  <<" "<<froll<<" "<<roll<<" "<<sroll<<endl
	  //  <<" "<<fheading<<" "<<heading<<" "<<sheading<<endl
	  //  <<" "<<flatitude<<" "<<latitude<<" "<<slatitude<<endl
	  //  <<" "<<flongitude<<" "<<longitude<<" "<<slongitude<<endl
	  //  <<" "<<faltitude<<" "<<altitude<<" "<<saltitude<<endl
	    <<" "<<flatitude<<" "<<latitude<<" "<<slatitude<<" "<<endl
	    <<" "<<flongitude<<" "<<longitude<<" "<<slongitude<<" "<<endl
	    <<" "<<faltitude<<" "<<altitude<<" "<<saltitude<<" "<<endl
	    <<" "<<fheading<<" "<<heading<<" "<<sheading<<" "<<endl
	    <<" "<<fpitch<<" "<<pitch<<" "<<spitch<<" "<<endl
	    <<" "<<froll<<" "<<roll<<" "<<sroll<<" "<<endl
	    <<" "<<fmrms<<" "<<mrms<<" "<<smrms<<" "<<endl
	    <<" "<<fbrms<<" "<<brms<<" "<<sbrms<<" "<<endl
	    <<" "<<fattFlag<<" "<<attFlag<<" "<<sattFlag<<endl
	    <<endl;
      }
      
      //Now need to create Adu5Pat object and fill tree
      interpAdu5Pat->run = doingRun;
      interpAdu5Pat->realTime = realTime;
      interpAdu5Pat->readTime = readTime;
      interpAdu5Pat->payloadTime=  payloadTime;
      interpAdu5Pat->payloadTimeUs =  payloadTimeUs;
      interpAdu5Pat->timeOfDay=  timeOfDay;
      interpAdu5Pat->latitude = latitude;
      interpAdu5Pat->longitude = longitude;
      interpAdu5Pat->altitude = altitude;
      interpAdu5Pat->heading = heading; 
      //interpAdu5Pat->pitch = pitch;      
      
      if(bestEntry!=bestEntryLast){
 	interpAdu5Pat->roll = froll;
 	interpAdu5Pat->pitch = fpitch;
	
      } else {
	interpAdu5Pat->pitch = pitch;
	interpAdu5Pat->roll = roll;
      }      


      interpAdu5Pat->mrms = mrms;
      interpAdu5Pat->brms = brms;
      interpAdu5Pat->attFlag = attFlag;
      interpAdu5Pat->intFlag = interpolationFlag;

      if(
	 theHeader->eventNumber==1076906||theHeader->eventNumber==1198819
	 ||
	 theHeader->eventNumber>1076806&&theHeader->eventNumber<1076905
	 //finterpolationFlag!=1||sinterpolationFlag!=1
	 ){
	cout<<endl<<bestEntry<<" "<<bestEntryLast<<" "<<theHeader->eventNumber<<" "<<theHeader->run<<endl
	    <<" "<<-frealTime+theHeader->triggerTime<<" -"<<srealTime-theHeader->triggerTime<<" "<<1e-9*Double_t(theHeader->triggerTimeNs)<<endl
	    <<" "<<finterpolationFlag<<" "<<interpAdu5Pat->intFlag<<" "<<sinterpolationFlag<<" "<<endl
	    <<" "<<-fpitch+interpAdu5Pat->pitch<<" "<<interpAdu5Pat->pitch-spitch<<" "<<endl
	    <<" "<<-froll+interpAdu5Pat->roll<<" "<<interpAdu5Pat->roll-sroll<<" "<<endl
	    <<" "<<-flatitude+interpAdu5Pat->latitude<<" "<<interpAdu5Pat->latitude-slatitude<<" "<<endl
	    <<" "<<-flongitude+interpAdu5Pat->longitude<<" "<<interpAdu5Pat->longitude-slongitude<<" "<<endl
	    <<" "<<-faltitude+interpAdu5Pat->altitude<<" "<<interpAdu5Pat->altitude-saltitude<<" "<<endl
	    <<" "<<-fheading+interpAdu5Pat->heading<<" "<<interpAdu5Pat->heading-sheading<<" "<<endl
	    <<" "<<-fpitch+interpAdu5Pat->pitch<<" "<<interpAdu5Pat->pitch-spitch<<" "<<endl
	    <<" "<<-froll+interpAdu5Pat->roll<<" "<<interpAdu5Pat->roll-sroll<<" "<<endl
	    <<" "<<-fmrms+interpAdu5Pat->mrms<<" "<<interpAdu5Pat->mrms-smrms<<" "<<endl
	    <<" "<<-fbrms+interpAdu5Pat->brms<<" "<<interpAdu5Pat->brms-sbrms<<" "<<endl
	    <<" "<<fattFlag<<" "<<interpAdu5Pat->attFlag<<" "<<sattFlag<<endl
	    <<" "<<-ftimeOfDay+interpAdu5Pat->timeOfDay<<" -"<<-interpAdu5Pat->timeOfDay+stimeOfDay<<" "<<endl
	    <<endl;
      }


      bestEntryLast = bestEntry;

      newTree->Fill();      
      
   }
   cerr << endl;
   newTree->AutoSave();
   newFp->Close();
}

