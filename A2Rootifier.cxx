#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

void A2Rootifier()
{

  TFile f("baseListA2.root","update");
  TTree A2Tree("A2Tree","TTree filled with ANITA-2 base locations");

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

  int indexFactor;

  A2Tree.Branch("name",&name);

  A2Tree.Branch("fullLat",&fullLat);
  
  A2Tree.Branch("longCar",&longCar);
  A2Tree.Branch("fullLong",&fullLong);

  A2Tree.Branch("indexFactor",&indexFactor);

  // CUSTOM VARS (calculated from the base list)
  
  // Load base list
  std::ifstream ifile("../data/convertedFiles/all_base_locations_new.txt");

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

      fullLat = std::stof(tokens[1]);
      fullLong = std::stof(tokens[3]);
      longCar = tokens[4];
      indexFactor = std::stoi(tokens[5]);

      fullLat = -fullLat;

      if(longCar == "W")
	{
	  fullLong = -fullLong;
	}

      A2Tree.Fill();

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

  A2Tree.Write();
  std::cout << "A2Tree written" << std::endl;
    

  return;
}
