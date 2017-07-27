#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

void antarcticTreatyRootifierA3()
{

  TFile f("baseListA3.root","update");
  TTree antarcticTreatyTree("antarcticTreatyTree","TTree filled with Antarctic Treaty refuges, stations and camps");

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
  std::string altCertainty;
  std::string primaryOperator;
  std::string facType;
  std::string seasonality;

  antarcticTreatyTree.Branch("name",&name);
  
  antarcticTreatyTree.Branch("latDeg",&latDeg,"latDeg/I");
  antarcticTreatyTree.Branch("latMin",&latMin,"latMin/D");
  antarcticTreatyTree.Branch("latCar",&latCar);
  antarcticTreatyTree.Branch("fullLat",&fullLat);
  
  antarcticTreatyTree.Branch("longDeg",&longDeg,"longDeg/I");
  antarcticTreatyTree.Branch("longMin",&longMin,"longMin/D");
  antarcticTreatyTree.Branch("longCar",&longCar);
  antarcticTreatyTree.Branch("fullLong",&fullLong);

  antarcticTreatyTree.Branch("alt",&alt);
  antarcticTreatyTree.Branch("altCertainty",&altCertainty);
  antarcticTreatyTree.Branch("primaryOperator",&primaryOperator);
  //antarcticTreatyTree.Branch("yearEst",&yearEst,"yearEst/I");
  antarcticTreatyTree.Branch("facType",&facType);
  antarcticTreatyTree.Branch("seasonality",&seasonality);

  // CUSTOM VARS (calculated from the base list)
  
  // Load base list
  std::ifstream ifile("./data/convertedFiles/base_list-unrestricted-A3.csv.1");

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
      latMin = std::stof(tokens[2]);
      latCar = tokens[3];
      longDeg = std::stoi(tokens[4]);
      longMin = std::stof(tokens[5]);
      longCar = tokens[6];
      alt = std::stof(tokens[7]);
      altCertainty = tokens[8];
      primaryOperator = tokens[9];
      facType = tokens[11];
      seasonality = tokens[12];

      fullLat = -latDeg - latMin/60;
      
      if(longCar == "E")
	{
	  fullLong = longDeg + (longMin)/60;
	}

      else if(longCar == "W")
	{
	  fullLong = -(longDeg + (longMin)/60);
	}

      antarcticTreatyTree.Fill();

      /*
      std::cout << " " << std::endl;
      std::cout << "\t Name: " << name << std::endl;
      std::cout << "\t Lat (deg): " << latDeg << std::endl;
      std::cout << "\t Lat (mins): " << latMin << std::endl;
      std::cout << "\t Lat cardinality: " << latCar << std::endl;
      std::cout << "\t Long (deg): " << longDeg << std::endl;
      std::cout << "\t Long (mins): " << longMin << std::endl;
      std::cout << "\t Long cardinality: " << longCar << std::endl;
      //std::cout << "\t Altitude: " << alt << std::endl;
      std::cout << "\t Primary operator: " << primaryOperator << std::endl;
      //std::cout << "\t Year established: " << yearEst << std::endl;
      std::cout << "\t Facility type: " << facType << std::endl;
      std::cout << "\t Seasonality: " << seasonality << std::endl;
      std::cout << "\t Current status: " << currentStatus << std::endl;
*/
      
    }

  antarcticTreatyTree.Write();
  std::cout << "antarcticTreatyTree written" << std::endl;
    

  return;
}
 
