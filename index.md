
# 15618 Project Proposal

Team member: Robert Truong \<hoangpht>

## Project website

Project website is available at [https://hoangphuoc25.github.io/15618-project](https://hoangphuoc25.github.io/15618-project)


# Parallel k-means clustering

## Summary

I will implement parallelized versions of the k-means clustering algorithm on different targets (CPU/GPU) and perform thorough analysis.

## Background

K-means clustering is an important unsupervised machine learning method that is being used widely in data mining to group data points together and potentially find something in common among the data in the same group.

## Challenges

Naive k-means doesn't guarantee convergence to global optimum, often depending on the initialization of centroids. There are variations of the naive algorithm that makes it easier to choose centroids (e.g., k-means++) and allows the algorithm to converge faster, and I'm planning to implement it as part of the optimization.

  

Another challenge is handling load imbalance. We have to handle all cases and aim to achieve a reasonable speedup in the case of heavily skewed data or lots of outliers.

## Resources

Hardware:

-   I will use GHC machines to test the implementations using CUDA/OpenMP, and possibly PSC machines if time permits an MPI implementation
    

Software:

-   I will implement the algorithm using C++ and OpenMP/CUDA/MPI
    

Test data:

-   I will use publicly available data, e.g. [this kaggle competition](https://www.kaggle.com/code/ryanholbrook/clustering-with-k-means), to validate the correctness of the sequential implementation and use it as the baseline to measure speedup of parallel implementations.
    

## Goals and deliverables

- 75% goal: minimum goal is to have a correct sequential implementation and complete parallel implementation with OpenMP and/or CUDA.

- 100% goal: I hope to achieve significant speedup compared to the sequential implementation, as well as a thorough analysis and comparison between the two implementations.

- 125% goal: if time permits and if I can find a large enough dataset, I hope to implement an MPI version and compare it against other parallel implementations.

## Schedule

11/9-11/13: Finalize project topic,

11/14-11/20: Research available literature, finish sequential implementation, generate large test cases for performance comparison

11/21-11/27: Complete first parallel implementation (OpenMP), and start optimization

11/28-12/3: Finish checkpoint, start second parallel implementation

12/4-12/10: Finish second parallel implementation, start optimization and perform thorough analysis

12/11-12/18: Finish report and demo

## Updates:
1. I will parallelize k-means clustering algorithm for the project.
2. The dataset used for performance benchmarking will be generated instead of collecting from public sources.

## Project midterm report

The midterm report is available [here](https://drive.google.com/file/d/1mPvtaXefN5sY9G1h6jy3d-97l_mSWNO1/view?usp=share_link)
