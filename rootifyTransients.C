/*  Macro written by John Russell in summer of 2017 to place transient data into ROOT binaries for ANITA 3 and 4.  To use it,
unpack the tar.gz files found under ANITA Log Book ELOG entry 703, labeling the directory of the unpacked contents
as "A3-list/" for the A3 data, "A4-list" for the A4 data. */


//  Function to ROOTify formatted AAD flight logs.
void rootifyAAD(TFile * transientFile, int &AVer) {
	
	//  Declarations to sift through.
	char callSign[8];
	int year, month, day, hour, minute, second, timeUTC, heading, altitude;
	double speed, fullLong, fullLat;
	
	//  Create the relevant TTree and its branches.
	TTree * AADTree = new TTree("AADTree", "AAD aircraft data");
	AADTree -> Branch("callSign", &callSign, "callSign/C");
	AADTree -> Branch("year", &year, "year/I");
	AADTree -> Branch("month", &month, "month/I");
	AADTree -> Branch("day", &day, "day/I");
	AADTree -> Branch("hour", &hour, "hour/I");
	AADTree -> Branch("minute", &minute, "minute/I");
	AADTree -> Branch("second", &second, "second/I");
	AADTree -> Branch("timeUTC", &timeUTC, "timeUTC/I");  // Converting date/time into epoch seconds.
	AADTree -> Branch("fullLat", &fullLat, "fullLat/D");
	AADTree -> Branch("fullLong", &fullLong, "fullLong/D");
	AADTree -> Branch("speed", &speed, "speed/D");
	AADTree -> Branch("heading", &heading, "heading/I");
	AADTree -> Branch("altitude", &altitude, "altitude/I");
	
	//  Name of file to read in, then open.
	string AStr = to_string(AVer);
	string AADFileName = "./A" + AStr + "_list/travel_logs-A" + AStr;
	AADFileName += "/AAD_aircraft-A" + AStr + "/AAD-aircraft_";
	AADFileName += (AVer == 3) ? "14-15-formatted.csv" : "16-formatted.csv";
	ifstream AADFile;
	AADFile.open(AADFileName.c_str());
	
	//  Permuting through the file, filling the tree.
	string headerLine;
	getline(AADFile, headerLine);  // To skip over the header line. Should also initialize stream.
	while (!AADFile.eof()) {
		
		AADFile >> callSign >> year >> month >> day >> hour >> minute >> second
			>> fullLat >> fullLong >> speed >> heading >> altitude;
		timeUTC = TTimeStamp(year, month, day, hour, minute, second);
		AADTree -> Fill();
	}
	
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
	char callSign[8], location[20];
	int year, month, day, hour, minute, timeUTC;
	double fullLong, fullLat, elevation;
	
	//  Create the relevant TTree and its branches.
	TTree * USAPFlightTree;
	if (restr == true) {
		USAPFlightTree = new TTree("USAPFlightRestrTree", "Restricted USAP flight log data");
		USAPFlightTree -> Branch("fullLat", &fullLat, "fullLat/D");
		USAPFlightTree -> Branch("fullLong", &fullLong, "fullLong/D");
		USAPFlightTree -> Branch("elevation", &elevation, "elevation/D");
	} else {
		USAPFlightTree = new TTree("USAPFlightUnrestrTree", "USAP flight log data");
	}
	USAPFlightTree -> Branch("callSign", &callSign, "callSign/C");
	USAPFlightTree -> Branch("location", &location, "location/C");
	USAPFlightTree -> Branch("year", &year, "year/I");
	USAPFlightTree -> Branch("month", &month, "month/I");
	USAPFlightTree -> Branch("day", &day, "day/I");
	USAPFlightTree -> Branch("hour", &hour, "hour/I");
	USAPFlightTree -> Branch("minute", &minute, "minute/I");
	USAPFlightTree -> Branch("timeUTC", &timeUTC, "timeUTC/I");  // Converting date/time into epoch seconds.
	
	//  Path to directory of files to be read in.
	string AStr = to_string(AVer);
	string USAPFlightFileName = "./A" + AStr + "_list/restricted-A" + AStr;
	USAPFlightFileName += "/USAP_flight_logs-restricted-A" + AStr + ".csv";
	
	//  Read in the restricted file.
	ifstream USAPFlightFile;
	USAPFlightFile.open(USAPFlightFileName.c_str());
	
	//  Permuting through the file, filling the tree.
	string headerLine;
	getline(USAPFlightFile, headerLine);  // To skip over the header line.
	while (!USAPFlightFile.eof()) {
		
		USAPFlightFile >> callSign >> year >> month >> day >> hour >> minute
					   >> location >> fullLat >> fullLong >> elevation;
		timeUTC = TTimeStamp(year, month, day, hour, minute, (int) 0);
		USAPFlightTree -> Fill();
	}
	
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
	int year, month, day, hour, minute, timeUTC;
	double degLat, minLat, fullLat, degLong, minLong, fullLong;
	
	//  Create the relevant TTree and its branches.
	TTree * USAPTraverseTree = new TTree("USAPTraverseTree", "USAP traverse data");
	USAPTraverseTree -> Branch("callSign", &callSign, "callSign/C");
	USAPTraverseTree -> Branch("year", &year, "year/I");
	USAPTraverseTree -> Branch("month", &month, "month/I");
	USAPTraverseTree -> Branch("day", &day, "day/I");
	USAPTraverseTree -> Branch("hour", &hour, "hour/I");
	USAPTraverseTree -> Branch("minute", &minute, "minute/I");
	USAPTraverseTree -> Branch("timeUTC", &timeUTC, "timeUTC/I");  //  Converting date/time into epoch seconds.
	USAPTraverseTree -> Branch("fullLat", &fullLat, "fullLat/D");
	USAPTraverseTree -> Branch("fullLong", &fullLong, "fullLong/D");
	
	//  Path to directory of files to be read in.
	string AStr = to_string(AVer);
	string USAPTraverseFileName = "./A" + AStr + "_list/travel_logs-A" + AStr;
	USAPTraverseFileName += "/USAP_traverse-A" + AStr;
	USAPTraverseFileName += "/USAP_traverse_coordinates-formatted-A" + AStr + ".csv";
	ifstream USAPTraverseFile;
	USAPTraverseFile.open(USAPTraverseFileName.c_str());
	
	//  Permuting through file, filling tree.
	string headerLine;
	getline(USAPTraverseFile, headerLine);  //  To skip over the header line.
	while (!USAPTraverseFile.eof()) {
		
		USAPTraverseFile >> callSign >> year >> month >> day;
		if (AVer == 3) {
			
			USAPTraverseFile >> hour >> minute;
			timeUTC = TTimeStamp(year, month, day, hour, minute, (int) 0);
			USAPTraverseFile >> fullLat >> cardLat >> fullLong >> cardLong;
		} else {
			
			timeUTC = TTimeStamp(year, month, day, (int) 0, (int) 0, (int) 0);
			USAPTraverseFile >> degLat >> minLat >> cardLat >> degLong >> minLong >> cardLong;
			fullLat = degLat + minLat / 60.;
			fullLong = degLong + minLong / 60.;
		}
		fullLat *= -1;  //  We're strictly southern hemisphere, so negative sign.
		fullLong *= (strncmp(&cardLong, "W", 1)) ? -1 : 1;
		
		USAPTraverseTree -> Fill();
	}
	
	// Close CSV file.
	USAPTraverseFile.close();
	
	// Write to USAPTraverseTree.
	transientFile -> cd();
	USAPTraverseTree -> Write();
}


//void rootifyUSAPTraverse(TFile * transientFile, int &AVer) {
//	
//	//  Declarations to sift through.
//	char callSign[8], cardLat, cardLong;
//	int year, month, day, hour, minute, timeUTC;
//	double degLat, minLat, fullLat, degLong, minLong, fullLong;
//	
//	//  Create the relevant TTree and its branches.
//	TTree * USAPTraverseTree = new TTree("USAPTraverseTree", "USAP traverse data");
//	USAPTraverseTree -> Branch("callSign", &callSign, "callSign/C");
//	USAPTraverseTree -> Branch("year", &year, "year/I");
//	USAPTraverseTree -> Branch("month", &month, "month/I");
//	USAPTraverseTree -> Branch("day", &day, "day/I");
//	USAPTraverseTree -> Branch("hour", &hour, "hour/I");
//	USAPTraverseTree -> Branch("minute", &minute, "minute/I");
//	USAPTraverseTree -> Branch("timeUTC", &timeUTC, "timeUTC/I");  //  Converting date/time into epoch seconds.
//	USAPTraverseTree -> Branch("fullLat", &fullLat, "fullLat/D");
//	USAPTraverseTree -> Branch("fullLong", &fullLong, "fullLong/D");
//	
//	//  Path to directory of files to be read in.
//	string AStr = to_string(AVer);
//	string USAPTraverseFilePath = "./A" + AStr + "_list/travel_logs-A" + AStr;
//	USAPTraverseFilePath += "/USAP_traverse-A" + AStr;
//	TSystemDirectory USAPTraverseDir(USAPTraverseFilePath.c_str(), USAPTraverseFilePath.c_str());
//	TList * USAPTraverseList = USAPTraverseDir.GetListOfFiles();
//	
//	//  Iterating through files.
//	TSystemFile * USAPTraverseTFile;
//	TIter USAPTraverseIter(USAPTraverseList);
//	while ((USAPTraverseTFile = (TSystemFile *) USAPTraverseIter())) {
//		
//		TString fileName = USAPTraverseFilePath + "/";
//		fileName += USAPTraverseTFile -> GetName();  //  GetName() will only list names in directory, not whole path.
//		if (fileName.EndsWith(".csv")) {
//			
//			ifstream USAPTraverseFile;
//			USAPTraverseFile.open(fileName.Data());  //  How to get a char variable from a TString.
//			string headerLine;
//			getline(USAPTraverseFile, headerLine);
//			while (!USAPTraverseFile.eof()) {
//				
//				if (AVer == 3) {
//					
//					USAPTraverseFile >> callSign >> year >> month >> day >> hour >> minute
//									 >> fullLat >> cardLat >> fullLong >> cardLong;
//				} else {
//					
//					USAPTraverseFile >> callSign >> year >> month >> day
//									 >> degLat >> minLat >> cardLat
//									 >> degLong >> minLong >> cardLong;
//					hour = 0;
//					minute = 0;
//					fullLat = degLat + minLat / 60;
//					fullLong = degLong + minLong / 60;
//				}
//				
//				timeUTC = TTimeStamp(year, month, day, hour, minute, (int) 0);	
//				fullLat *= -1;  //  We're strictly southern hemisphere, so negative sign.
//				fullLong *= (strncmp(&cardLong, "W", 1)) ? -1 : 1;
//				
//				USAPTraverseTree -> Fill();
//			}
//			
//			// Close CSV file.
//			USAPTraverseFile.close();
//		}
//	}
//	
//	// Write to USAPTraverseTree.
//	transientFile -> cd();
//	USAPTraverseTree -> Write();
//}


/*  Function to ROOTify additional data provided by other sources, currently from Konstantin and
	an Icetrek expedition leader.  */
void rootifyFeedback(TFile * transientFile, int &AVer) {
	
	//  Currently, only additional data for ANITA-4.
	if (AVer != 4) return;
	
	//  Declarations to sift through.
	char callSign[8], location[20], comment[40];
	int year, month, day, timeUTC;
	double fullLong, fullLat;
	
	//  Create the relevant TTree and its branches.
	TTree * feedbackTree = new TTree("feedbackTree", "Feedback on transients data");
	feedbackTree -> Branch("callSign", &callSign, "callSign/C", 8);
	feedbackTree -> Branch("location", &location, "location/C", 20);
	feedbackTree -> Branch("comment", &comment, "comment/C", 40);
	feedbackTree -> Branch("year", &year, "year/I");
	feedbackTree -> Branch("month", &month, "month/I");
	feedbackTree -> Branch("day", &day, "day/I");
	feedbackTree -> Branch("timeUTC", &timeUTC, "timeUTC/I");  // Converting date/time into epoch seconds.
	feedbackTree -> Branch("fullLat", &fullLat, "fullLat/D");
	feedbackTree -> Branch("fullLong", &fullLong, "fullLong/D");
	
	// Icetrek feedback data. Start.
	strcpy(callSign, "Icetrek");
	strcpy(location, "Ross_Ice_Shelf");
	strcpy(comment, "Via Reedy Glacier. Estimating.");
	year = 2016;
	month = 12;
	day = 7;
	timeUTC = TTimeStamp(year, month, day, (int) 0, (int) 0, (int) 0);
	fullLat = -81.5;
	fullLong = -175;
	feedbackTree -> Fill();
		
	// Icetrek feedback data. Finish.
	strcpy(callSign, "Icetrek");
	strcpy(location, "South_Pole");
	strcpy(comment, "Via Reedy Glacier. Assuming station.");
	year = 2017;
	month = 1;
	day = 11;
	timeUTC = TTimeStamp(year, month, day, (int) 0, (int) 0, (int) 0);
	fullLat = -89 - 59.85 / 60.;
	fullLong = 139 + 16.368 / 60.;
	feedbackTree -> Fill();
	
	// Konstantin's feedback data for Russian sledge-caterpillar traverse (SCT). Start.
	strcpy(callSign, "SCT");
	strcpy(location, "Progress");
	strcpy(comment, "None.");
	year = 2016;
	month = 12;
	day = 3;
	timeUTC = TTimeStamp(year, month, day, (int) 0, (int) 0, (int) 0);
	fullLat = -69 - 24 / 60.;
	fullLong = 76 + 24 / 60.;
	feedbackTree -> Fill();
	
	// Konstantin feedback data. Finish.
	strcpy(callSign, "SCT");
	strcpy(location, "Vostok");
	strcpy(comment, "None.");
	year = 2016;
	month = 12;
	day = 7;
	timeUTC = TTimeStamp(year, month, day, (int) 0, (int) 0, (int) 0);
	fullLat = -78 - 28 / 60.;
	fullLong = 106 + 48 / 60.;
	feedbackTree -> Fill();
	
	// Write to feedbackTree.
	transientFile -> cd();
	feedbackTree -> Write();
}


//  Master fuction which calls all others.
void rootifyTransients(int AVer = 3, bool restr = false) {

	//  Recreate ROOT file with ROOTified data.
	TString fileName = "transientList";
	fileName += (restr == false) ? "Unrestricted" : "Restricted";
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
