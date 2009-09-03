#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <stdio.h>

#include "TFile.h"
#include "TTree.h"
#include "TTimeStamp.h"

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
  for(int i = 0; i<inBranch.size(); i++){
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
  sprintf(headName,"/unix/anita1/flight0809/root/run%d/headFile%d.root",doingRun,doingRun); 
  TFile *fHead= new TFile(headName,"OLD");
  TTree *headTree = (TTree*) fHead->Get("headTree");
  
  RawAnitaHeader *theHeader=0;
  headTree->SetBranchAddress("header",&theHeader);

  //ANITA II has a different format than ANITA I, and the 
  //gps files are already made
  char gpsName[FILENAME_MAX]; 
  sprintf(gpsName,"/unix/anita1/flight0809/root/run%d/gpsFile%d.root",doingRun,doingRun); 
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
   sprintf(outName,"/unix/anita1/flight0809/root/run%d/gpsEvent%d.root",doingRun,doingRun);  
   //sprintf(outName,"/raid2/grashorn/anita/runTreeMaker/gpsEvent%d.root",doingRun);  
   TFile *newFp= new TFile(outName,"RECREATE"); 
   
   TTree *newTree= new TTree("adu5PatTree","Tree of Interpolated ADU5 Positions and Attitude");;
   Adu5Pat *interpAdu5Pat = new Adu5Pat();
   newTree->Branch("pat","Adu5Pat",&interpAdu5Pat);
   int bestEntryLast=0;
   
   //create a log file in case something goes wrong.  If nothing goes wrong, delete empty log file
   char filename[FILENAME_MAX];
   sprintf(filename,"log/log_gpsEvent%d.txt",doingRun,doingRun);
   ofstream fileout;
   fileout.open(filename);
   
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
   vector<double> headingVec;


   double lastHeading=0;
   //fill pitch and roll vectors
   for(int i = 0; i < hkEntries; i++){
     adu5PatTree->GetEntry(i);
     if(adu5PatPtr->heading>=0 && adu5PatPtr->heading<=360) {
	headingVec.push_back(adu5PatPtr->heading);
	lastHeading=adu5PatPtr->heading;
     }
     else {
	headingVec.push_back(lastHeading);
     }
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

   //Crossing from longitude 180 to -180 poses a problem for the interpolator.
   //So, lets identify these situations and do long+=360 if long<0 for these
   //situations, do the interpolation, then subtract the long-=360    
   int boundaryCrossing = 0;
   adu5PatTree->GetEntry(0);
   double firstLong = adu5PatPtr->longitude;
   cout<<adu5PatPtr->timeOfDay<<endl;
   adu5PatTree->GetEntry(hkEntries-1);
   double lastLong = adu5PatPtr->longitude;
   if(lastLong/firstLong<0) boundaryCrossing=1;
   
   int fbad = 0;
   int sbad = 0;
   Long64_t nbytes = 0;
   Long64_t starEvery=nentries/20;
   int timeDiffOk = 120;
   int timeDiff = 0;
   if(starEvery==0) starEvery=1;
    
   for (Long64_t i=0; i<nentries;i++) {
     
     fbad = 0;
     sbad = 0;
     
     if(i%starEvery==0) cerr << "*";
     
     nbytes += headTree->GetEntry(i);
     Long64_t bestEntry=adu5PatTree->GetEntryNumberWithBestIndex(theHeader->triggerTime,theHeader->triggerTimeNs/1000);

     //this check is hopefully unnecessary.  I'd swear i saw it happen once, though, so i'm keeping it just to be safe.
     if(bestEntry==hkEntries) {
       bestEntry--;
       adu5PatTree->GetEntry(bestEntry);

       if((theHeader->triggerTime-adu5PatPtr->payloadTime)>timeDiffOk) {
	 bestEntry=-1; 
	 fileout<<"last GPS packet "<<theHeader->triggerTime-adu5PatPtr->payloadTime<<" older than event.  Skipping. ";
       }
     }

     
     //if bestEntry has a value of -1, this means that the event time is 
     //earlier than the first time in the GPS data.  Within a certain range,
     //this is still acceptable.  The timeDiffOk variable is set above.
     adu5PatTree->GetEntry(bestEntry);
     if(bestEntry<0) adu5PatTree->GetEntry(0);
     timeDiff = (double)theHeader->triggerTime-(double)adu5PatPtr->payloadTime;
     if(bestEntry<0&&fabs(timeDiff)<timeDiffOk) bestEntry = 0;

     if(bestEntry<0) {
       fileout<< "Something bad happened in entry: " << i 
	      << " bestEntry value: "<<bestEntry;
       fileout<<";  HeaderTimestamp:"
	      <<TTimeStamp(theHeader->triggerTime).AsString("s")
	      <<" GPS Timestamp:"
	      <<TTimeStamp(adu5PatPtr->payloadTime).AsString("s")<<timeDiff<<endl;

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
      
       adu5PatTree->GetEntry(bestEntry);
       
       //Copy to temporary variables
       frealTime=adu5PatPtr->realTime;
       fpayloadTime=adu5PatPtr->payloadTime;
       freadTime=adu5PatPtr->payloadTime;
       ftimeOfDay=adu5PatPtr->timeOfDay;
       flatitude=adu5PatPtr->latitude;
       flongitude=adu5PatPtr->longitude;

	if(boundaryCrossing==1&&flongitude<0)flongitude+=360;
	if(flongitude==0) {
	  fileout<<"There's a bad value here: "<<theHeader->eventNumber
		 <<" flongitude=="<<flongitude<<endl;
	  fbad=1;
	}
	
	faltitude=adu5PatPtr->altitude;
	fheading=headingVec.at(bestEntry);
	//fpitch=adu5PatPtr->pitch;
	if(bestEntry<ssPitchVec.size()) fpitch=ssPitchVec.at(bestEntry);
	else { 
	  fpitch=adu5PatPtr->pitch;
	  fileout<<"bestEntry ("<<bestEntry<< ") exceeds size of ssPitchVec ("
		 <<ssPitchVec.size()<<"); fpitch not smoothed"<<endl;
	}
	if(bestEntry<ssRollVec.size()) froll=ssRollVec.at(bestEntry);
	else { 
	  froll=adu5PatPtr->roll;
	  fileout<<"bestEntry ("<<bestEntry<< ") exceeds size of ssRollVec ("
		 <<ssRollVec.size()<<"); froll not smoothed"<<endl;
	}
	//froll=adu5PatPtr->roll;
	fmrms=adu5PatPtr->mrms;
	fbrms=adu5PatPtr->brms;
	fattFlag=adu5PatPtr->attFlag;
	//bestEntry++;
	if(bestEntry<hkEntries-1) bestEntry++;
	else fileout<<"event index "<<i<<", GPS index "<<bestEntry
		    <<", GPSSize " <<hkEntries
		    <<" last data point in GPS file; cannot interpolate"<<endl;
	adu5PatTree->GetEntry(bestEntry);
	
	//Copy to temporary variables
	srealTime=adu5PatPtr->realTime;
	stimeOfDay=adu5PatPtr->timeOfDay;
	slatitude=adu5PatPtr->latitude;
	slongitude=adu5PatPtr->longitude;

	if(boundaryCrossing==1&&slongitude<0)slongitude+=360;
	if(slongitude==0) {
	  fileout<<" There's a bad value here: "<<theHeader->eventNumber
		 <<" slongitude=="<<slongitude<<endl;
	  sbad=1;
	}

	saltitude=adu5PatPtr->altitude;
	//	sheading=adu5PatPtr->heading;
	sheading=headingVec.at(bestEntry);
	//spitch=adu5PatPtr->pitch;
	if(bestEntry<ssPitchVec.size()) spitch=ssPitchVec.at(bestEntry);
	else { 
	  spitch=adu5PatPtr->pitch;
	  fileout<<"at event "<<i<<", bestEntry ("<<bestEntry
		 << ") exceeds size of ssPitchVec ("
		 <<ssPitchVec.size()<<"); spitch not smoothed"<<endl;
	}
	if(bestEntry<ssRollVec.size()) sroll=ssRollVec.at(bestEntry);
	else { 
	  sroll=adu5PatPtr->roll;
	  fileout<<"at event "<<i<<", bestEntry ("<<bestEntry
		 << ") exceeds size of ssRollVec ("
		 <<ssRollVec.size()<<"); sroll not smoothed"<<endl;
	
	}
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

	if(finterpolationFlag*60<sinterpolationFlag||sbad==1) {
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
	else if(sinterpolationFlag*60<finterpolationFlag||fbad==1) {
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
	    if(TMath::Abs(360+sheading-fheading)<TMath::Abs(sheading-fheading)) {
	      sheading+=360;
	    }
	    if(TMath::Abs(sheading-(fheading+360))<TMath::Abs(sheading-fheading)) {
	      fheading+=360;
	    }
	    
	    heading=fheading + timeFactor*(sheading-fheading);
	    if(heading>=360) {
	      heading-=360;
	    } 


	    //	    heading=fheading + timeFactor*(sheading-fheading);
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

    
     //set boundary crossing values correctly
     if(boundaryCrossing==1&&longitude>180.)longitude-=360;
     
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
      //if(i<200)cout<<i<<" "<<pitch<<" "<<fpitch<<" "<<roll<<" "<<froll<<endl;      
      
      interpAdu5Pat->mrms = mrms;
      interpAdu5Pat->brms = brms;
      interpAdu5Pat->attFlag = attFlag;
      interpAdu5Pat->intFlag = interpolationFlag;
      
      
      bestEntryLast = bestEntry;

      newTree->Fill();      
      
   }
   cerr << endl;
   newTree->AutoSave();
   newFp->Close();
}

