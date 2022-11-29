#include "world.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>

bool World::loadData(std::string fileName) {
  std::ifstream inFile;
  inFile.open(fileName);
  if (!inFile) {
    return false;
  }

  std::string line;
  std::cout << "inFile:" << fileName << std::endl;
  while (std::getline(inFile, line)) {
    Point point;
    std::stringstream sstream(line);
    std::string str;
    std::getline(sstream, str, '\t');
    point.id = (int)atoi(str.c_str());
    std::getline(sstream, str, '\t');
    point.x = (float)atof(str.c_str());
    std::getline(sstream, str, '\t');
    point.y = (float)atof(str.c_str());
    point.id = (int)data.size();
    data.push_back(point);
  }
  inFile.close();
  cluster.resize(data.size());
  return true;
}
bool World::loadCentroids(std::string fileName) {
  std::ifstream inFile;
  inFile.open(fileName);
  if (!inFile) {
    return false;
  }

  std::string line;
  std::cout << "inFile:" << fileName << std::endl;
  while (std::getline(inFile, line)) {
    Point point;
    std::stringstream sstream(line);
    std::string str;
    std::getline(sstream, str, '\t');
    point.x = (float)atof(str.c_str());
    std::getline(sstream, str, '\t');
    point.y = (float)atof(str.c_str());
    point.id = (int)data.size();
    centroids.push_back(point);
  }
  inFile.close();
  return true;
}