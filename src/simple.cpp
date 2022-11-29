#include "world.h"
#include "timing.h"
#include <algorithm>
#include <iostream>

#include <omp.h>
#include <numeric>
#include <math.h>

float distance(Point p1, Point p2) {
  return sqrt(pow(p1.x-p2.x, 2) + pow(p1.y-p2.y, 2));
}

class SimpleKMeansRunner : public IKMeansRunner {
public:
  virtual void run(std::vector<Point> &data,
                  std::vector<Point> &centroids,
                  std::vector<int> &labels,
                  std::vector<Point> &newCentroids,
                  TimeCost &times) override {
    float *count = new float[centroids.size()];
    float *sumX = new float[centroids.size()];
    float *sumY = new float[centroids.size()];

    Timer timer;
    for (size_t i=0; i < data.size(); i++) {
      float minDistance;
      int group = 0;
      for (size_t j=0; j < centroids.size(); j++) {
        float dist = distance(data[i], centroids[j]);
        if (dist < minDistance) {
          group = j;
          minDistance = dist;
        }
      }
      // if (i%1000 == 0) {
      //   printf("%d\n", i);
      // }
      labels.push_back(group);
      count[group] += 1;
      sumX[group] += data[i].x;
      sumY[group] += data[i].y;
    }
    times.labellingTime += timer.elapsed();

    timer.reset(); 
    for (size_t i=0; i < centroids.size(); i++) {
      newCentroids.push_back(Point{.id=static_cast<int>(i), .x=sumX[i]/count[i], .y=sumY[i]/count[i]});
    }
    times.newCentroidTime += timer.elapsed();
  }
};

std::unique_ptr<IKMeansRunner> createSimpleRunner() {
  return std::make_unique<SimpleKMeansRunner>();
}
