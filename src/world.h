#ifndef WORLD_H_
#define WORLD_H_

#include <math.h>
#include <memory>
#include <vector>
#include <iostream>
#include <ostream>
#include <cstring>
#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string.h>

struct Point {
    int id;
    double x;
    double y;
};

struct TimeCost {
  double labellingTime = 0;
  double newCentroidTime = 0;
};

class IKMeansRunner {
public:
    virtual void run(std::vector<Point> &points,
                    std::vector<Point> &centroids,
                    std::vector<int> &labels,
                    std::vector<Point> &newCentroids,
                    TimeCost &times) = 0;
    virtual ~IKMeansRunner() {}
};

class World {
public:
  std::vector<Point> data;
  std::vector<int> cluster;
  std::unique_ptr<IKMeansRunner> kMeansRunner;
  std::vector<Point> centroids;

  bool loadData(std::string fileName);
  bool loadCentroids(std::string fileName);
  void exportLabel(std::string fileName);
};




std::unique_ptr<IKMeansRunner> createSimpleRunner();
std::unique_ptr<IKMeansRunner> createOMPRunner();
std::unique_ptr<IKMeansRunner> createCUDARunner();
std::unique_ptr<IKMeansRunner> createMPIRunner();


inline bool loadFromFile(std::string fileName,
                         std::vector<Point> &data,
                         std::vector<int> &assignment) {
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
    std::getline(sstream, str);
    
    point.id = (int)atoi(str.c_str());
    std::getline(sstream, str, '\t');
    point.x = (float)atof(str.c_str());
    std::getline(sstream, str, '\t');
    point.y = (float)atof(str.c_str());
    point.id = (int)data.size();
    data.push_back(point);
  }
  assignment.resize(data.size());
  inFile.close();
  return true;
}

inline bool loadCentroids(std::string fileName, std::vector<Point>& centroids) {
  std::ifstream inFile;
  inFile.open(fileName);
  if (!inFile) {
    return false;
  }

  std::string line;
  std::cout << "inFile:" << fileName << std::endl;
  int id=0;
  while (std::getline(inFile, line)) {
    Point point;
    std::stringstream sstream(line);
    std::string str;
    std::getline(sstream, str, '\t');
    point.x = (float)atof(str.c_str());
    std::getline(sstream, str, '\t');
    point.y = (float)atof(str.c_str());
    point.id = id;
    id++;
    centroids.push_back(point);
  }
  inFile.close();
  return true;
}

float distance(Point p1, Point p2);

#endif