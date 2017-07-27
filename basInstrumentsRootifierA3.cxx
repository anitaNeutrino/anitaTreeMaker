#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

void basInstrumentsRootifierA3()
{

  TFile f("baseListA3.root","update");
  TTree basInstrumentsTree("basInstrumentsTree","TTree filled with BAS instrument sites");

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

  basInstrumentsTree.Branch("name",&name);
  
  basInstrumentsTree.Branch("latDeg",&latDeg,"latDeg/I");
  basInstrumentsTree.Branch("latMin",&latMin,"latMin/D");
  basInstrumentsTree.Branch("latCar",&latCar);
  basInstrumentsTree.Branch("fullLat",&fullLat);
  
  basInstrumentsTree.Branch("longDeg",&longDeg,"longDeg/I");
  basInstrumentsTree.Branch("longMin",&longMin,"longMin/D");
  basInstrumentsTree.Branch("longCar",&longCar);
  basInstrumentsTree.Branch("fullLong",&fullLong);

  basInstrumentsTree.Branch("alt",&alt);

  // CUSTOM VARS (calculated from the bas list)
  
  // Load base list
  std::ifstream ifile("../data/convertedFiles/base_list-unrestricted-A3.csv.4");

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

      fullLat = -latDeg - latMin/60;
      
      if(longCar == "E")
	{
	  fullLong = longDeg + (longMin)/60;
	}

      else if(longCar == "W")
	{
	  fullLong = -(longDeg + (longMin)/60);
	}

      basInstrumentsTree.Fill();

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
*/
      
    }

  basInstrumentsTree.Write();
  std::cout << "basInstrumentsTree written" << std::endl;
    

  return;
}
