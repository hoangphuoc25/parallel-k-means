import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns
from sklearn.cluster import KMeans
import numpy as np

from sklearn.datasets import make_blobs

NO_CLUSTER=12 
X, y_true = make_blobs(n_samples=1600000, centers=NO_CLUSTER,
                       cluster_std=6
                       , random_state=0, center_box=(-100, 100))

plt.style.use("seaborn-whitegrid")
plt.rc("figure", autolayout=True)
plt.rc(
    "axes",
    labelweight="bold",
    labelsize="large",
    titleweight="bold",
    titlesize=14,
    titlepad=10,
)

plt.scatter(X[:, 0], X[:, 1], s=2)

kmeans = KMeans(n_clusters=NO_CLUSTER)                                                                                                       
kmeans.fit(X)
centroids = kmeans.cluster_centers_
plt.scatter( 
    centroids[:, 0], 
    centroids[:, 1], 
    marker="x", 
    s=169, 
    linewidths=1, 
    color="b", 
    zorder=10, 
)

kmeans = KMeans(n_clusters=NO_CLUSTER) 
kmeans.fit(X)
y_kmeans = kmeans.predict(X)

plt.scatter(X[:, 0], X[:, 1], c=y_kmeans, s=1, cmap='viridis')
centers = kmeans.cluster_centers_
plt.scatter(centers[:, 0], centers[:, 1], c='red', s=20, alpha=0.5)
# plt.savefig('dataset.png')

np.savetxt('small.csv', X, delimiter='\t', fmt='%2.5f')
np.savetxt('small_centroids.csv', centers, delimiter='\t', fmt='%2.5f')
