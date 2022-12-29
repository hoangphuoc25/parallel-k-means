#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string.h>
#include "world.h"
#include "mpi.h"
#include "timing.h"
#include <algorithm>

struct StartupOptions {
  int numIterations = 1;
  int numCluster = 1;
  float spaceSize = 10.0f;
  int numPoints = 5;
  std::string outputFile = "out.txt";
  std::string inputFile;
  std::string centroidFile;
  bool checkCorrectness = false;
  bool useOMP = false;
  bool useMPI = false;
  bool useCUDA = false;
};


std::string removeQuote(std::string input) {
  if (input.length() > 0 && input.front() == '\"')
    return input.substr(1, input.length() - 2);
  return input;
}

StartupOptions parseOptions(int argc, const char **argv) {
  StartupOptions rs;
  for (int i = 1; i < argc; i++) {
    if (i < argc - 1) {
      if (strcmp(argv[i], "-k") == 0)
        rs.numCluster = atoi(argv[i + 1]);
      else if (strcmp(argv[i], "-i") == 0)
        rs.numIterations = atoi(argv[i + 1]);
      else if (strcmp(argv[i], "-s") == 0)
        rs.spaceSize = (float)atof(argv[i + 1]);
      else if (strcmp(argv[i], "-c") == 0)
        rs.checkCorrectness = true;
      else if (strcmp(argv[i], "-in") == 0)
        rs.inputFile = removeQuote(argv[i + 1]);
      else if (strcmp(argv[i], "-ce") == 0)
        rs.centroidFile = removeQuote(argv[i + 1]);
      else if (strcmp(argv[i], "-n") == 0)
        rs.numPoints = atoi(argv[i + 1]);
      else if (strcmp(argv[i], "-o") == 0)
        rs.outputFile = argv[i + 1];
      else if (strcmp(argv[i], "-omp") == 0)
        rs.useOMP = true;
      else if (strcmp(argv[i], "-mpi") == 0)
        rs.useMPI = true;
      else if (strcmp(argv[i], "-cuda") == 0)
        rs.useCUDA = true;
    }
  }
  return rs;
}

int main(int argc, const char **argv) {
  StartupOptions options = parseOptions(argc, argv);
  if (!options.useMPI) {
    World w;
    World refW;
    if (options.inputFile.length()) {
      w.loadData(options.inputFile);
    }
    if (options.centroidFile.length()) {
      w.loadCentroids(options.centroidFile);
    }
    for (int i = 0; i < 10; i++) {
      printf("%f %f\n", w.centroids[i].x, w.centroids[i].y);
    }
    w.kMeansRunner = createSimpleRunner();
    if (options.useOMP) {
      w.kMeansRunner = createOMPRunner();
    }

    printf("data.size=%d centroids.size=%d\n", w.data.size(), w.centroids.size());
    
    for (int i=0; i < options.numIterations; i++) {
      std::vector<int> labels;
      labels.resize(w.data.size());
      std::vector<Point> newCentroids;
      TimeCost tc;
      w.kMeansRunner -> run(
        w.data,
        w.centroids,
        labels,
        newCentroids,
        tc
      );
      printf("elapsed: %f %f \n", tc.labellingTime, tc.newCentroidTime);
    }

    return 0;
  }

  int pid;
  int nproc;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &pid);
  MPI_Comm_size(MPI_COMM_WORLD, &nproc);

  int sizeTag = 1;
  int dataTag = 2;
  int recvSize = 0;

  World w;
  World refW;
  if (options.inputFile.length()) {
    w.loadData(options.inputFile);
  }
  if (options.centroidFile.length()) {
    w.loadCentroids(options.centroidFile);
  }

  int *count = new int[w.centroids.size()];
  for (int i=0; i < w.centroids.size(); i++) {
    count[i] = 0;
  }
  float sumX = 0.0f;
  float sumY = 0.0f;

  std::vector<Point> ownPoints;


  for (size_t i=0; i < w.data.size(); i++) {
    float minDistance=distance(w.data[i], w.centroids[0]);
    int label = 0;
    for (size_t j=0; j < w.centroids.size(); j++) {
      float dist = distance(w.data[i], w.centroids[j]);
      if (dist < minDistance) {
        label = j;
        minDistance = dist;
      }
    }
    if (label == pid) {
      ownPoints.push_back(w.data[i]);
      sumX += w.data[i].x;
      sumY += w.data[i].y;
    }
  }

  for (int iterCount=0; iterCount < options.numIterations; iterCount++) {
    Timer timer;
    Point newCentroid = Point{.x = sumX/ownPoints.size(), .y=sumY/ownPoints.size()};

    int* sizes = new int[options.numCluster];
    for (int i =0; i < options.numCluster; i++) {
      sizes[i] = sizeof(Point);
    }
    int* displacements = new int[options.numCluster];
    displacements[0] = 0;
    for (int j=0; j<options.numCluster-1; j++){
      displacements[j+1] = displacements[j] + sizes[j];
    }
    std::vector<Point> nextCentroids(options.numCluster);
    MPI_Allgatherv(&newCentroid, sizeof(Point), MPI_BYTE,
      nextCentroids.data(), sizes, displacements, MPI_BYTE, MPI_COMM_WORLD);
    
    
    std::vector<std::vector<Point>> belongToOtherCluster(options.numCluster);
    std::vector<Point> newOwnPoints;
    
    sumX = 0.0f;
    sumY = 0.0f;
    for (int i=0; i < ownPoints.size(); i++) {
      float minDistance=distance(ownPoints[i], nextCentroids[0]);
      int label = 0;
      for (size_t j=0; j < w.centroids.size(); j++) {
        float dist = distance(ownPoints[i], w.centroids[j]);
        if (dist < minDistance) {
          label = j;
          minDistance = dist;
        }
      }
      if (label == pid) {
        newOwnPoints.push_back(ownPoints[i]);
        sumX += ownPoints[i].x;
        sumY += ownPoints[i].y;
      } else {
        belongToOtherCluster[label].push_back(ownPoints[i]);
      }
    }
    
    std::vector<int> sendSizes(options.numCluster);
    for (int i=0; i < options.numCluster; i++) {
      sendSizes[i] = belongToOtherCluster[i].size();
    }

    MPI_Request reqs[options.numCluster-1];
    int reqIdx = 0;
    for (int i=0; i < options.numCluster; i++) {
      if (i == pid) {
        continue;
      }
      MPI_Isend(sendSizes.data()+i, 1, MPI_INT, i, sizeTag, MPI_COMM_WORLD, &reqs[reqIdx++]);
    }
    MPI_Waitall(options.numCluster-1, reqs, MPI_STATUSES_IGNORE);

    std::vector<int> recvSizeBuffer(options.numCluster);
    std::vector<Point> recvBuffer;
    for (int i=0; i < options.numCluster; i++) {
      
      recvBuffer.clear();
      if (i == pid) {
        continue;
      }
      MPI_Recv(&recvSizeBuffer[i],1,MPI_INT, i, sizeTag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    int countReqs2 = 0;
    for (int i=0; i < options.numCluster; i++) {
      if (sendSizes[i] > 0) {
        countReqs2+=1;
      }
    }
    
    if (countReqs2 > 0) {
      MPI_Request reqs2[countReqs2];
      reqIdx = 0;
      for (int i=0; i < options.numCluster; i++) {
        if (sendSizes[i] > 0) {
          // printf("pid=%d sending %d to %d\n", pid, belongToOtherCluster[i].size(), i);
          MPI_Isend(belongToOtherCluster[i].data(), belongToOtherCluster[i].size()*sizeof(Point), 
            MPI_BYTE, i, dataTag, MPI_COMM_WORLD, &reqs2[reqIdx++]);
        }
      }
      MPI_Waitall(countReqs2, reqs2, MPI_STATUSES_IGNORE);
    }
    
    int maxBufferSize = *std::max_element(recvSizeBuffer.begin(), recvSizeBuffer.end());
    std::vector<Point> newPointRecvBuffer(maxBufferSize);
    for (int i=0; i < options.numCluster; i++) {
      if (recvSizeBuffer[i] > 0) {
        newPointRecvBuffer.clear();
        MPI_Recv(newPointRecvBuffer.data(), recvSizeBuffer[i]*sizeof(Point), MPI_BYTE, i, dataTag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("pid=%d iter=%d received %d from %d\n", pid, iterCount, recvSizeBuffer[i], i);
        newOwnPoints.insert(newOwnPoints.end(), newPointRecvBuffer.begin(), newPointRecvBuffer.begin()+recvSizeBuffer[i]);
      }
    }
    ownPoints = newOwnPoints;
    if (pid == 0) {
      printf("elapsed: %f \n", timer.elapsed());
    }
  }

  MPI_Finalize();

}