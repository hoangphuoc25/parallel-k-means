import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns
from sklearn.cluster import KMeans

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

df = pd.read_csv("airbnb.csv")
X = df.loc[:, ["latitude", "longitude"]]
# print(X.head())
NO_CLUSTER=4
kmeans = KMeans(n_clusters=NO_CLUSTER)
centroids = [[]]
X["Cluster"] = kmeans.fit_predict(X)
X["Cluster"] = X["Cluster"].astype("category")
X.head()
X.to_csv(f'airbnb_cluster_{NO_CLUSTER}.csv', columns=['Cluster'])
print(kmeans.cluster_centers_)

plt.scatter(df['latitude'], df['longitude'], color='k', s=2)
centroids = kmeans.cluster_centers_
plt.scatter( 
    centroids[:, 0], 
    centroids[:, 1], 
    marker="x", 
    s=169, 
    linewidths=3, 
    color="b", 
    zorder=10, 
)
plt.show()
# [[ 40.65156128 -73.98645335]
#  [ 40.77824405 -73.94500643]
#  [ 40.69483911 -73.93928288]
#  [ 40.73686943 -73.98885484]
#  [ 40.706829   -73.81671982]
#  [ 40.83527086 -73.92780289]]