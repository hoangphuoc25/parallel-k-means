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
      // else if (strcmp(argv[i], "-ref") == 0)
      //   rs.referenceAnswerDir = removeQuote(argv[i + 1]);
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

  

  // if (options.checkCorrectness) {
  //   std::cout << "Correctness Checking Enabled";
  //   refW.nbodySimulator = createSimpleNBodySimulator();
  //   if (options.inputFile.length())
  //     refW.loadFromFile(options.inputFile);
  //   else
  //     refW.loadFromFile("reference-init.txt");
  // }

  // std::string simulatorName;

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
  // for (int i = 0; i < 10; i++) {
  //   printf("ct: %f %f\n", w.centroids[i].x, w.centroids[i].y);
  // }
  // for (int i = 0; i < 10; i++) {
  //   printf("datA: %f %f\n", w.data[i].x, w.data[i].y);
  // }
  // printf("centroids size %d\n", w.centroids.size());

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
    // w.cluster[i] = label;
    // count[label] += 1;
  }
  // printf("pid=%d ownSize=%d\n", pid, ownPoints.size());

  for (int iterCount=0; iterCount < options.numIterations; iterCount++) {
    Timer timer;
    Point newCentroid = Point{.x = sumX/ownPoints.size(), .y=sumY/ownPoints.size()};
    // printf("pid=%d newCentroid=%f %f iter=%d\n", pid, newCentroid.x, newCentroid.y, iterCount);

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
    // printf("belongToOtherCluster pid=%d ", pid);
    // for (int i = 0;i < options.numCluster; i++) {
    //   printf("%d ", belongToOtherCluster[i].size());
    // }
    // printf("\n");
    std::vector<int> sendSizes(options.numCluster);
    for (int i=0; i < options.numCluster; i++) {
      sendSizes[i] = belongToOtherCluster[i].size();
    }
    // printf("sendSizes pid=%d ", pid);
    // for (int i=0; i < options.numCluster; i++) {
    //   printf("%d ", sendSizes[i]);
    // }
    // printf("\n");

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

    // printf("recvSizeBuffer pid=%d ", pid);
    // for (int i=0; i < options.numCluster; i++) {
    //   printf("%d ", recvSizeBuffer[i]);
    // }
    // printf("\n");

    int countReqs2 = 0;
    for (int i=0; i < options.numCluster; i++) {
      if (sendSizes[i] > 0) {
        countReqs2+=1;
      }
    }
    // printf("pid=%d iter=%d countReqs2=%d\n",pid, iterCount,countReqs2);
    
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
    
    // printf("pid=%d iter=%d before maxbuffersize\n", pid, iterCount);
    int maxBufferSize = *std::max_element(recvSizeBuffer.begin(), recvSizeBuffer.end());
    // printf("pid=%d iter=%d maxBufferSize=%d\n", pid, iterCount, maxBufferSize);
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
    // printf("pid=%d finished iter=%d\n", pid, iterCount);
    if (pid == 0) {
      printf("elapsed: %f \n", timer.elapsed());
    }
    // for (int i=0; i < newOwnPoints.size(); i++) {
    //   sumX += newOwnPoints[i].x; 
    //   sumY += newOwnPoints[i].y;
    // }
  }
  // printf("pid=%d after=%d\n", pid, newOwnPoints.size());

  // printf("mpirecv pid=%d ", pid);
  // for (int i=0; i < options.numCluster; i++) {
  //   printf("%d ", recvSizeBuffer[i]);
  // }
  // printf("\n");

  


  // MPI_Request reqs[options.numCluster-1];
  // for (int i=0;i < options.numCluster; i++) {
  //   printf("%d ", count[i]);
  // }
  // printf("\n");

  // for (int i = 1; i < options.numCluster; i++) {
  //   MPI_Isend(&count[i], 1, MPI_INT, i, sizeTag, MPI_COMM_WORLD, &reqs[i-1]);
  // }
  // MPI_Waitall(options.numCluster-1, reqs, MPI_STATUSES_IGNORE);
  // MPI_Request reqs2[options.numCluster-1];
  // for (int clusterId=1; clusterId < options.numCluster; clusterId++) {
  //   std::vector<Point> initialCluster(count[i]);
  //   for (int pointId=0; pointId < w.data.size(); pointId++) {
  //     if (w.cluster[pointId] == clusterId) {
  //       initialCluster.append(w.data[pointId])
  //     }
  //   }
  // }

  
  MPI_Finalize();
  return 0;

  // switch (options.simulatorType) {
  // case SimulatorType::Simple:
  //   w.nbodySimulator = createSimpleNBodySimulator();
  //   simulatorName = "Simple";
  //   break;
  // case SimulatorType::Sequential:
  //   w.nbodySimulator = createSequentialNBodySimulator();
  //   simulatorName = "Sequential";
  //   break;
  // case SimulatorType::Parallel:
  //   w.nbodySimulator = createParallelNBodySimulator();
  //   simulatorName = "Parallel";
  //   break;
  // }
  // std::cout << simulatorName << "\n";
  // StepParameters stepParams;
  // stepParams = getBenchmarkStepParams(options.spaceSize);

  // // run the implementation
  // bool fullCorrectness = true;
  // TimeCost totalTimeCost;
  // for (int i = 0; i < options.numIterations; i++) {
  //   TimeCost timeCost;
  //   TimeCost timeCostRef;
  //   w.simulateStep(stepParams, timeCost);
  //   totalTimeCost.treeBuildingTime += timeCost.treeBuildingTime;
  //   totalTimeCost.simulationTime += timeCost.simulationTime;
  //   if (options.checkCorrectness) {
  //     refW.simulateStep(stepParams, timeCostRef);
  //     bool correct = checkForCorrectness(simulatorName, refW, w, "",
  //                                        options.numParticles, stepParams);
  //     if (correct != true)
  //       fullCorrectness = false;
  //   }
  //   // displayIterationPerformance(i, timeCost);

  //   // generate simulation image
  //   if (options.frameOutputStyle == FrameOutputStyle::AllFrames) {
  //     std::stringstream sstream;
  //     sstream << options.bitmapOutputDir;
  //     if (!options.bitmapOutputDir.size() ||
  //         (options.bitmapOutputDir.back() != '\\' &&
  //          options.bitmapOutputDir.back() != '/'))
  //       sstream << "/";
  //     sstream << i << ".bmp";
  //     w.dumpView(sstream.str(), options.viewportRadius);
  //   }
  // }
  // displayTotalPerformance(options.numIterations, totalTimeCost);

  // if (options.outputFile.length()) {
  //   w.saveToFile(options.outputFile);
  // }
  // return !fullCorrectness;
}