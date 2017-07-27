#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

void awsRootifierA3()
{

  TFile f("baseListA3.root","update");
  TTree awsTree("awsTree","TTree filled with Antarctic Weather Stations");

  // BASE LIST VARS:
  std::string name;
  
  int latDeg;
  double latMin;
  std::string latCar;
  double fullLat;
  
  int longDeg;
  double longMin;
  std::string longCar;
  double fullLong;
  
  double alt; // will crash, as altitude isn't filled for all fields
  int realTime; // perhaps convert this to unixTime
  std::string currentStatus;

  awsTree.Branch("name",&name);
  
  awsTree.Branch("latDeg",&latDeg,"latDeg/I");
  awsTree.Branch("latMin",&latMin,"latMin/D");
  awsTree.Branch("latCar",&latCar);
  awsTree.Branch("fullLat",&fullLat);
  
  awsTree.Branch("longDeg",&longDeg,"longDeg/I");
  awsTree.Branch("longMin",&longMin,"longMin/D");
  awsTree.Branch("longCar",&longCar);
  awsTree.Branch("fullLong",&fullLong);

  awsTree.Branch("alt",&alt);
  awsTree.Branch("realTime",&realTime,"realTime/I");
  awsTree.Branch("currentStatus",&currentStatus);

  // CUSTOM VARS (calculated from the base list)
  
  // Load base list
  std::ifstream ifile("../data/convertedFiles/base_list-unrestricted-A3.csv.3");

  std::string line; 
  while (std::getline(ifile, line))
    {
      
      std::istringstream iss(line); // construct a string stream from line

      // read the tokens from current line separated by comma
      std::vector<std::string> tokens; // here we store the tokens
      std::string token; // current token
      while (std::getline(iss, token, ','))
        {
	  tokens.push_back(token); // add the token to the vector
        }

      // map the tokens into our variables
      name = tokens[0];
      latDeg = std::stoi(tokens[1]);
      // Skip millidegrees
      latMin = std::stof(tokens[3]);
      latCar = tokens[4];
      longDeg = std::stoi(tokens[5]);
       // Skip millidegrees
      longMin = std::stof(tokens[7]);
      longCar = tokens[8];
      alt = std::stof(tokens[9]);

      // Convert date to realTime (unixTime)
      struct tm tm;
      time_t ts = 0;
      memset(&tm, 0, sizeof(tm));
      std::string humanTime = tokens[10];
      strptime(humanTime.c_str(), "%Y/%m/%d", &tm);
      ts = mktime(&tm);
      realTime = (int)ts;
      
      currentStatus = tokens[11];

      fullLat = -latDeg - latMin/60;
      
      if(longCar == "E")
	{
	  fullLong = longDeg + (longMin)/60;
	}

      else if(longCar == "W")
	{
	  fullLong = -(longDeg + (longMin)/60);
	}

      awsTree.Fill();

      /*
      std::cout << " " << std::endl;
      std::cout << "\t Name: " << name << std::endl;
      std::cout << "\t Lat (deg): " << latDeg << std::endl;
      std::cout << "\t Lat (mins): " << latMin << std::endl;
      std::cout << "\t Lat cardinality: " << latCar << std::endl;
      std::cout << "\t Long (deg): " << longDeg << std::endl;
      std::cout << "\t Long (mins): " << longMin << std::endl;
      std::cout << "\t Long cardinality: " << longCar << std::endl;
      std::cout << "\t Altitude: " << alt << std::endl;
      std::cout << "\t Year established: " << yearEst << std::endl;
      std::cout << "\t Current status: " << currentStatus << std::endl;
      */
      
    }

  awsTree.Write();
  std::cout << "awsTree written" << std::endl;
    

  return;
}
