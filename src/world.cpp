#include "world.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <algorithm>

bool compare(Point a, Point b) {
  if (a.x != b.x) {
    return a.x < b.x;
  }
  return a.y < b.y;
}

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
    point.x = (float)atof(str.c_str());
    std::getline(sstream, str, '\t');
    point.y = (float)atof(str.c_str());
    point.id = (int)data.size();
    data.push_back(point);
  }
  inFile.close();
  cluster.resize(data.size());
  // std::sort(data.begin(), data.end(), compare);
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