OUTPUTDIR := bin/

CFLAGS := -std=c++14 -O2 -fvisibility=hidden -lpthread -Wall -Wextra -fpermissive
# CFLAGS := -std=c++14 -g -O0 -fvisibility=hidden -lpthread -Wall -Wextra -fpermissive

ifeq (,$(CONFIGURATION))
	CONFIGURATION := release
endif

ifeq (debug,$(CONFIGURATION))
CFLAGS += -g -fopenmp
else
CFLAGS += -O2 -fopenmp
endif

SOURCES := src/*.cpp
HEADERS := src/*.h
MAIN_SOURCES := src/simple.cpp src/omp.cpp src/world.cpp

CXX = mpic++

.SUFFIXES:
.PHONY: all clean

all: kmeans

kmeans: $(HEADERS) $(SOURCES)
	$(CXX) -o $@ $(CFLAGS) $(HEADERS) src/simple.cpp src/omp.cpp src/world.cpp src/main.cpp


clean:
	rm -rf ./kmeans