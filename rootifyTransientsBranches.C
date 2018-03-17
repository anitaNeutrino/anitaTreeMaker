/*  Macro written by John Russell in summer of 2017 to place transient data into ROOT binaries for ANITA 3 and 4. Places TLeaves into TBranches corresponding to vehicle's call sign.  To use it, unpack the tar.gz files found under ANITA Log Book ELOG entry 703, labeling the directory of the unpacked contents as "A3-list/" for the A3 data, "A4-list" for the A4 data.*/


//  Function to ROOTify formatted AAD flight logs.
void rootifyAAD(TFile * transientFile, int &AVer) {
	
	//  Declarations to sift through.
	char callSign[8];
	struct {
		
		int year, month, day, hour, minute, second, timeUTC;
		double fullLat, fullLong, speed;
		int heading, altitude;
	} AADStruct;
	
	//  Create the relevant TTree. Branches to be defined conditionally.
	TTree * AADTree = new TTree("AADTree", "AAD aircraft data");
	
	//  Name of file to read in, then open.
	string AStr = to_string(AVer);
	string AADFileName = "./A" + AStr + "_list/travel_logs-A" + AStr;
	AADFileName += "/AAD_aircraft-A" + AStr + "/AAD-aircraft_";
	AADFileName += (AVer == 3) ? "14-15" : "16";
	AADFileName += "-branch_formatted.csv";
	ifstream AADFile;
	AADFile.open(AADFileName.c_str());
	
	//  Permuting through the file, filling the tree.
	string headerLine;
	getline(AADFile, headerLine);  //  To skip over the header line. Should also initialize stream.
	while (!AADFile.eof()) {
		
		AADFile >> callSign;
		if (!AADTree -> GetListOfBranches() -> Contains(callSign)) {
			AADTree -> Branch(callSign, &AADStruct, "year/I:month:day:hour:minute:second:timeUTC:fullLat/D:fullLong:speed:heading/I:altitude");
		}
		
		AADFile >> AADStruct.year >> AADStruct.month >> AADStruct.day >> AADStruct.hour >> AADStruct.minute >> AADStruct.second;
		AADStruct.timeUTC = TTimeStamp(AADStruct.year, AADStruct.month, AADStruct.day, AADStruct.hour, AADStruct.minute, AADStruct.second);
		AADFile	>> AADStruct.fullLat >> AADStruct.fullLong >> AADStruct.speed >> AADStruct.heading >> AADStruct.altitude;
		AADTree -> GetBranch(callSign) -> Fill();
	}
	
	//  Reformatting for the branches inside the tree.
	AADTree -> SetEntries();
	
	//  Close opened AADFile.
	AADFile.close();
	
	//  Write AADTree to transientFile.
	transientFile -> cd();
	AADTree -> Write();
}


/*  Function to ROOTify formatted USAP flight logs. Optional arguement to ROOTify restricted
    data available.  */
void rootifyUSAPFlight(TFile * transientFile, int &AVer, bool &restr) {
	
	//  Declarations to sift through.
	char callSign[8];
	struct {
		
		int year, month, day, hour, minute, timeUTC;
		char location[20];
		double fullLat, fullLong, elevation;
	} USAPFlightStruct;
	double fullLat, fullLong, elevation;
	
	//  Create the relevant TTree and its branches.
	TTree * USAPFlightTree;
	if (restr == true) {
		USAPFlightTree = new TTree("USAPFlightRestrTree", "Restricted USAP flight log data");
	} else {
		USAPFlightTree = new TTree("USAPFlightUnrestrTree", "USAP flight log data");
	}
	
	//  Path to directory of files to be read in.
	string AStr = to_string(AVer);
	string USAPFlightFileName = "./A" + AStr + "_list/restricted-A" + AStr;
	USAPFlightFileName += "/USAP_flight_logs-restricted-branch-A" + AStr + ".csv";
	
	//  Read in the restricted file.
	ifstream USAPFlightFile;
	USAPFlightFile.open(USAPFlightFileName.c_str());
	
	//  Permuting through the file, filling the tree.
	string headerLine;
	getline(USAPFlightFile, headerLine);  //  To skip over the header line.
	while (!USAPFlightFile.eof()) {
	
		USAPFlightFile >> callSign;
		if (!USAPFlightTree -> GetListOfBranches() -> Contains(callSign)) {
			
			if (restr == true) {
				
				USAPFlightTree -> Branch(callSign, &USAPFlightStruct, "year/I:month:day:hour:minute:timeUTC:location[20]/C:fullLat/D:fullLong:elevation");
			} else {
				
				USAPFlightTree -> Branch(callSign, &USAPFlightStruct, "year/I:month:day:hour:minute:timeUTC:location[20]/C");
			}
		}
		USAPFlightFile >> USAPFlightStruct.year >> USAPFlightStruct.month >> USAPFlightStruct.day >> USAPFlightStruct.hour >> USAPFlightStruct.minute;
		USAPFlightStruct.timeUTC = TTimeStamp(USAPFlightStruct.year, USAPFlightStruct.month, USAPFlightStruct.day, USAPFlightStruct.hour, USAPFlightStruct.minute, (int) 0);
		USAPFlightFile >> USAPFlightStruct.location;
		if (restr == true) {
			
			USAPFlightFile >> USAPFlightStruct.fullLat >> USAPFlightStruct.fullLong >> USAPFlightStruct.elevation;
		} else {
			
			USAPFlightFile >> fullLat >> fullLong >> elevation;
		}
		USAPFlightTree -> GetBranch(callSign) -> Fill();
	}

	//  Reformatting for the branches inside the tree.
	USAPFlightTree -> SetEntries();
	
	//  Closing opened USAPFlightFile.
	USAPFlightFile.close();
			
	//  Writing USAPFlightTree to transientFile.
	transientFile -> cd();
	USAPFlightTree -> Write();
}


//  Function to ROOTify formatted USAP traverse locations.
void rootifyUSAPTraverse(TFile * transientFile, int &AVer) {
	
	//  Declarations to sift through.
	char callSign[8], cardLat, cardLong;
	double degLat, minLat, degLong, minLong;
	struct {
		
		int year, month, day, hour, minute, timeUTC;
		double fullLat, fullLong;
		
	} USAPTraverseStruct;
	
	//  Create the relevant TTree and its branches.
	TTree * USAPTraverseTree = new TTree("USAPTraverseTree", "USAP traverse data");
	
	//  Path to directory of files to be read in.
	string AStr = to_string(AVer);
	string USAPTraverseFileName = "./A" + AStr + "_list/travel_logs-A" + AStr;
	USAPTraverseFileName += "/USAP_traverse-A" + AStr;
	USAPTraverseFileName += "/USAP_traverse_coordinates-branch_formatted-A" + AStr + ".csv";
	ifstream USAPTraverseFile;
	USAPTraverseFile.open(USAPTraverseFileName.c_str());
	
	//  Permuting through file, filling tree.
	string headerLine;
	getline(USAPTraverseFile, headerLine);  //  To skip over the header line.
	while (!USAPTraverseFile.eof()) {
	
		USAPTraverseFile >> callSign;
		if (!USAPTraverseTree -> GetListOfBranches() -> Contains(callSign)) {
			
			USAPTraverseTree -> Branch(callSign, &USAPTraverseStruct, "year/I:month:day:hour:minute:timeUTC:fullLat/D:fullLong");
		}
		USAPTraverseFile >> USAPTraverseStruct.year >> USAPTraverseStruct.month >> USAPTraverseStruct.day;
		if (AVer == 3) {
			
			USAPTraverseFile >> USAPTraverseStruct.hour >> USAPTraverseStruct.minute;
			USAPTraverseStruct.timeUTC = TTimeStamp(USAPTraverseStruct.year, USAPTraverseStruct.month, USAPTraverseStruct.day, USAPTraverseStruct.hour, USAPTraverseStruct.minute, (int) 0);
			USAPTraverseFile >> USAPTraverseStruct.fullLat >> cardLat >> USAPTraverseStruct.fullLong >> cardLong;

		} else {
			
			USAPTraverseStruct.hour = 0;
			USAPTraverseStruct.minute = 0;
			USAPTraverseStruct.timeUTC = TTimeStamp(USAPTraverseStruct.year, USAPTraverseStruct.month, USAPTraverseStruct.day, USAPTraverseStruct.hour, USAPTraverseStruct.minute, (int) 0);
			USAPTraverseFile >> degLat >> minLat >> cardLat >> degLong >> minLong >> cardLong;
			USAPTraverseStruct.fullLat = degLat + minLat / 60.;
			USAPTraverseStruct.fullLong = degLong + minLong / 60.;
		}
		USAPTraverseStruct.fullLat *= -1;  //  We're strictly southern hemisphere, so negative sign.
		USAPTraverseStruct.fullLong *= (strncmp(&cardLong, "W", 1)) ? -1 : 1;
		
		USAPTraverseTree -> GetBranch(callSign) -> Fill();
	}
	
	//  Reformatting for the branches inside the tree.
	USAPTraverseTree -> SetEntries();
	
	//  Closing opened file.
	USAPTraverseFile.close();
	
	// Write to USAPTraverseTree.
	transientFile -> cd();
	USAPTraverseTree -> Write();
}


/*  Function to ROOTify additional data provided by other sources, currently from Konstantin and
	an Icetrek expedition leader.  */
void rootifyFeedback(TFile * transientFile, int &AVer) {
	
	//  Currently, only additional data for ANITA-4.
	if (AVer != 4) return;
	
	//  Declarations to sift through.
	struct {
		int year, month, day, timeUTC;
		char location[20];
		double fullLat, fullLong;
		char comment[40];
	} feedbackStruct;
	
	//  Create the relevant TTree.
	TTree * feedbackTree = new TTree("feedbackTree", "Feedback on transients data");

	if (AVer == 3) {

		//  Konstantin's feedback data for sledge-caterpillar traverse (SCT).
		feedbackTree -> Branch("SCT", &feedbackStruct, "year/I:month:day:timeUTC:location[20]/C:fullLat/D:fullLong:comment[40]/C");
		//  Konstantin start.
		feedbackStruct.year = 2014;
		feedbackStruct.month = 10;
		feedbackStruct.day = 26;
		feedbackStruct.timeUTC = TTimeStamp(feedbackStruct.year, feedbackStruct.month, feedbackStruct.day, (int) 0, (int) 0, (int) 0);
		strcpy(feedbackStruct.location, "Progress");
		feedbackStruct.fullLat = -69 - 24 / 60.;
		feedbackStruct.fullLong = 76 + 24 / 60.;
		strcpy(feedbackStruct.comment, "None.");
		feedbackTree -> GetBranch("SCT") -> Fill();

		feedbackStruct.year = 2014;
		feedbackStruct.month = 12;
		feedbackStruct.day = 10;
		feedbackStruct.timeUTC = TTimeStamp(feedbackStruct.year, feedbackStruct.month, feedbackStruct.day, (int) 0, (int) 0, (int) 0);
		strcpy(feedbackStruct.location, "Vostok");
		feedbackStruct.fullLat = -78 - 28 / 60.;
		feedbackStruct.fullLong = 106 + 48 / 60.;
		strcpy(feedbackStruct.comment, "None.");
		feedbackTree -> GetBranch("SCT") -> Fill();

		feedbackStruct.year = 2014;
		feedbackStruct.month = 12;
		feedbackStruct.day = 14;
		feedbackStruct.timeUTC = TTimeStamp(feedbackStruct.year, feedbackStruct.month, feedbackStruct.day, (int) 0, (int) 0, (int) 0);
		strcpy(feedbackStruct.location, "Vostok");
		feedbackStruct.fullLat = -78 - 28 / 60.;
		feedbackStruct.fullLong = 106 + 48 / 60.;
		strcpy(feedbackStruct.comment, "None.");
		feedbackTree -> GetBranch("SCT") -> Fill();
		// Konstantin finish.
		feedbackStruct.year = 2014;
		feedbackStruct.month = 12;
		feedbackStruct.day = 21;
		feedbackStruct.timeUTC = TTimeStamp(feedbackStruct.year, feedbackStruct.month, feedbackStruct.day, (int) 0, (int) 0, (int) 0);
		strcpy(feedbackStruct.location, "Progress");
		feedbackStruct.fullLat = -69 - 24 / 60.;
		feedbackStruct.fullLong = 76 + 24 / 60.;
		strcpy(feedbackStruct.comment, "None.");
		feedbackTree -> GetBranch("SCT") -> Fill();
} else if (AVer == 4) {

		//  Icetrek feedback data.
		feedbackTree -> Branch("Icetrek", &feedbackStruct, "year/I:month:day:timeUTC:location[20]/C:fullLat/D:fullLong:comment[40]/C");
		//  Icetrek start.
		feedbackStruct.year = 2016;
		feedbackStruct.month = 12;
		feedbackStruct.day = 7;
		feedbackStruct.timeUTC = TTimeStamp(feedbackStruct.year, feedbackStruct.month, feedbackStruct.day, (int) 0, (int) 0, (int) 0);
		strcpy(feedbackStruct.location, "Ross_Ice_Shelf");
		feedbackStruct.fullLat = -81.5;
		feedbackStruct.fullLong = -175;
		strcpy(feedbackStruct.comment, "Via Reedy Glacier. Estimating.");
		feedbackTree -> GetBranch("Icetrek") -> Fill();
		// Icetrek finish.
		feedbackStruct.year = 2017;
		feedbackStruct.month = 1;
		feedbackStruct.day = 11;
		feedbackStruct.timeUTC = TTimeStamp(feedbackStruct.year, feedbackStruct.month, feedbackStruct.day, (int) 0, (int) 0, (int) 0);
		strcpy(feedbackStruct.location, "South_Pole");
		feedbackStruct.fullLat = -89 - 59.85 / 60.;
		feedbackStruct.fullLong = 139 + 16.368 / 60.;
		strcpy(feedbackStruct.comment, "Via Reedy Glacier. Assuming station.");
		feedbackTree -> GetBranch("Icetrek") -> Fill();

		//  Konstantin's feedback data for sledge-caterpillar traverse (SCT).
		feedbackTree -> Branch("SCT", &feedbackStruct, "year/I:month:day:timeUTC:location[20]/C:fullLat/D:fullLong:comment[40]/C");
		//  Konstantin start.
		feedbackStruct.year = 2016;
		feedbackStruct.month = 10;
		feedbackStruct.day = 18;
		feedbackStruct.timeUTC = TTimeStamp(feedbackStruct.year, feedbackStruct.month, feedbackStruct.day, (int) 0, (int) 0, (int) 0);
		strcpy(feedbackStruct.location, "Progress");
		feedbackStruct.fullLat = -69 - 24 / 60.;
		feedbackStruct.fullLong = 76 + 24 / 60.;
		strcpy(feedbackStruct.comment, "None.");
		feedbackTree -> GetBranch("SCT") -> Fill();

		feedbackStruct.year = 2016;
		feedbackStruct.month = 10;
		feedbackStruct.day = 30;
		feedbackStruct.timeUTC = TTimeStamp(feedbackStruct.year, feedbackStruct.month, feedbackStruct.day, (int) 0, (int) 0, (int) 0);
		strcpy(feedbackStruct.location, "Vostok");
		feedbackStruct.fullLat = -78 - 28 / 60.;
		feedbackStruct.fullLong = 106 + 48 / 60.;
		strcpy(feedbackStruct.comment, "None.");
		feedbackTree -> GetBranch("SCT") -> Fill();

		feedbackStruct.year = 2016;
		feedbackStruct.month = 12;
		feedbackStruct.day = 3;
		feedbackStruct.timeUTC = TTimeStamp(feedbackStruct.year, feedbackStruct.month, feedbackStruct.day, (int) 0, (int) 0, (int) 0);
		strcpy(feedbackStruct.location, "Vostok");
		feedbackStruct.fullLat = -78 - 28 / 60.;
		feedbackStruct.fullLong = 106 + 48 / 60.;
		strcpy(feedbackStruct.comment, "None.");
		feedbackTree -> GetBranch("SCT") -> Fill();
		// Konstantin finish.
		feedbackStruct.year = 2016;
		feedbackStruct.month = 12;
		feedbackStruct.day = 14;
		feedbackStruct.timeUTC = TTimeStamp(feedbackStruct.year, feedbackStruct.month, feedbackStruct.day, (int) 0, (int) 0, (int) 0);
		strcpy(feedbackStruct.location, "Progress");
		feedbackStruct.fullLat = -69 - 24 / 60.;
		feedbackStruct.fullLong = 76 + 24 / 60.;
		strcpy(feedbackStruct.comment, "None.");
		feedbackTree -> GetBranch("SCT") -> Fill();
	} else return;

	
	//  Reformatting for the branches inside the tree.
	feedbackTree -> SetEntries();
	
	// Write to feedbackTree.
	transientFile -> cd();
	feedbackTree -> Write();
}


//  Master fuction which calls all others.
void rootifyTransientsBranches(int AVer = 3, bool restr = false) {
	
	//  Recreate ROOT file with ROOTified data.
	TString fileName = "transientList";
	fileName += (restr == false) ? "Unrestricted" : "Restricted";
	fileName += "Branched";
	fileName += "A" + TString::Itoa(AVer, 10) + ".root";
	TFile * transientFile = new TFile(fileName, "RECREATE");
	
	//  Use other functions to populate transientFile with TTrees.
	rootifyAAD(transientFile, AVer);
	rootifyUSAPFlight(transientFile, AVer, restr);
	rootifyUSAPTraverse(transientFile, AVer);
	rootifyFeedback(transientFile, AVer);
		
	//  Close transientFile.
	transientFile -> Close();
}
