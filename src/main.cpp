#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string.h>
#include "world.h"

struct StartupOptions {
  int numIterations = 1;
  int numCluster = 1;
  float spaceSize = 10.0f;
  int numPoints = 5;
  std::string outputFile = "out.txt";
  std::string inputFile;
  std::string centroidFile;
  bool checkCorrectness = false;
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
      // else if (strcmp(argv[i], "-ref") == 0)
      //   rs.referenceAnswerDir = removeQuote(argv[i + 1]);
    }
  }
  return rs;
}

int main(int argc, const char **argv) {
  StartupOptions options = parseOptions(argc, argv);

  World w;
  World refW;
  if (options.inputFile.length()) {
    w.loadData(options.inputFile);
  }
  if (options.centroidFile.length()) {
    w.loadCentroids(options.centroidFile);
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
  for (int i = 0; i < 10; i++) {
    printf("%f %f\n", w.centroids[i].x, w.centroids[i].y);
  }
  w.kMeansRunner = createSimpleRunner();
  for (int i=0; i < options.numIterations; i++) {
    std::vector<int> labels;
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
