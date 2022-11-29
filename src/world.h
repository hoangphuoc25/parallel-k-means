#include <math.h>
#include <memory>
#include <vector>

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
std::unique_ptr<IKMeansRunner> createOMPRuner();
std::unique_ptr<IKMeansRunner> createCUDARunner();
std::unique_ptr<IKMeansRunner> createMPIRunner();
