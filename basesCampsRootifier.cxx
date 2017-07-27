#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

void basesCampsRootifier()
{

  TFile f("baseList.root","update");
  TTree baseCampTree("baseCampTree","TTree filled with research bases and camps around Antarctica");

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
  
  double alt;
  std::string altCertainty;
  std::string primaryOperator;
  int yearEst;
  std::string facType;
  std::string seasonality;
  std::string currentStatus;

  baseCampTree.Branch("name",&name);
  
  baseCampTree.Branch("latDeg",&latDeg,"latDeg/I");
  baseCampTree.Branch("latMin",&latMin,"latMin/D");
  baseCampTree.Branch("latCar",&latCar);
  baseCampTree.Branch("fullLat",&fullLat);
  
  baseCampTree.Branch("longDeg",&longDeg,"longDeg/I");
  baseCampTree.Branch("longMin",&longMin,"longMin/D");
  baseCampTree.Branch("longCar",&longCar);
  baseCampTree.Branch("fullLong",&fullLong);

  baseCampTree.Branch("alt",&alt);
  baseCampTree.Branch("altCertainty",&altCertainty);
  baseCampTree.Branch("primaryOperator",&primaryOperator);
  baseCampTree.Branch("yearEst",&yearEst,"yearEst/I");
  baseCampTree.Branch("facType",&facType);
  baseCampTree.Branch("seasonality",&seasonality);
  baseCampTree.Branch("currentStatus",&currentStatus);

  // CUSTOM VARS (calculated from the base list)
  
  // Load base list
  std::ifstream ifile("./data/convertedFiles/base_list-unrestricted-A4.csv.0");

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
      yearEst = std::stoi(tokens[10]);
      facType = tokens[11];
      seasonality = tokens[12];
      currentStatus = tokens[13];

      fullLat = -latDeg - latMin/60;
      
      if(longCar == "E")
	{
	  fullLong = longDeg + (longMin)/60;
	}

      else if(longCar == "W")
	{
	  fullLong = -(longDeg + (longMin)/60);
	}

      baseCampTree.Fill();

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

  baseCampTree.Write();
  std::cout << "baseCampTree written" << std::endl;
    

  return;
}
