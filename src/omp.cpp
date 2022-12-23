#include "world.h"
#include "timing.h"
#include <algorithm>
#include <iostream>

#include <omp.h>
#include <numeric>
#include <math.h>

float distance2(Point p1, Point p2) {
  return sqrt(pow(p1.x-p2.x, 2) + pow(p1.y-p2.y, 2));
}

class OMPKMeansRunner : public IKMeansRunner {
public:
  virtual void run(std::vector<Point> &data,
                  std::vector<Point> &centroids,
                  std::vector<int> &labels,
                  std::vector<Point> &newCentroids,
                  TimeCost &times) override {
    float *count = new float[centroids.size()];
    float *sumX = new float[centroids.size()];
    float *sumY = new float[centroids.size()];

    for (size_t i=0; i < centroids.size(); i++) {
      count[i] = 0;
      sumX[i] = 0.0f;
      sumY[i] = 0.0f;
    }

    Timer timer;
    printf("with omp\n");
    int datasize = data.size();
    int centroidsize = centroids.size();

    #pragma omp parallel for schedule(static) reduction(+:sumX[:centroidsize]) reduction(+:sumY[:centroidsize]) reduction(+:count[:centroidsize]) 
    for (size_t i=0; i < datasize; i++) {
      float minDistance=distance2(data[0], centroids[0]);
      int group = 0;
      for (size_t j=0; j < centroidsize; j++) {
        float dist = distance2(data[i], centroids[j]);
        if (dist < minDistance) {
          group = j;
          minDistance = dist;
        }
      }
      labels[i] = group;
      sumX[group] += data[i].x;
      sumY[group] += data[i].y;
      count[group] += 1;
    } // end for
  
    times.labellingTime += timer.elapsed();
    for (size_t i=0; i < centroids.size(); i++) {
      newCentroids.push_back(Point{.id=static_cast<int>(i), .x=sumX[i]/count[i], .y=sumY[i]/count[i]});
    }

    delete [] count;
    delete [] sumX;
    delete [] sumY;
  }
};

std::unique_ptr<IKMeansRunner> createOMPRunner() {
  return std::make_unique<OMPKMeansRunner>();
}
