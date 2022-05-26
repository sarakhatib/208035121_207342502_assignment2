import sys
import math
import numpy as np
import pandas as pd
import mykmeanssp

class Point:
    def __init__(self, d, coordinates):
        self.d=d
        self.coordinates=coordinates
        self.clusterIndex = -1

    def set_clusterIndex(self , index):
        self.clusterIndex=index
        #print("nice")
    
    def __repr__(self):
        res = "Coordinates: " + str(self.coordinates) + " Cluster index: " + str(self.clusterIndex)
        return res

class Cluster:
    def __init__(self,centroid,index):
        self.centroid_coordinates=centroid
        self.index=index
        self.len=0
        self.norm=0

    def set_centroid(self,centroid):
        self.centroid_coordinates=centroid
    
    def set_len(self,len):
        self.len=len

    def set_norm(self,norm):
        self.norm=norm
    
    def __repr__(self):
        res = "Centroid: " + str(self.centroid_coordinates) + " norm: " + str(self.norm) + " len:" + str(self.len)
        return res
    

def main():
    n = len(sys.argv)
    k=0 
    max_iter = 200 
    if(not sys.argv[1].isdigit()):
        print("Invalid Input!")
        return 1
    if(n==5):
        k = int(sys.argv[1])
        epsilon = sys.argv[2]
        file_name_1 = sys.argv[3]
        file_name_2 = sys.argv[4]
    elif(n==6):
        k = int(sys.argv[1])
        if(not sys.argv[2].isdigit() or int(sys.argv[2])<1):
            print("Invalid Input!")
            return 1
        max_iter = int(sys.argv[2])
        epsilon = sys.argv[3]
        file_name_1 = sys.argv[4]
        file_name_2 = sys.argv[5]
    else:
        print("Invalid Input!")
        return 1
    if(k<1):
        print("Invalid Input!")
        return 1
    epsilon = float(epsilon)
    iter_num=0
    Euclidean_Norm=True
    data_1 = pd.read_csv(file_name_1, header=None)
    data_1 = data_1.sort_values(0)
    data_2 = pd.read_csv(file_name_2, header=None)
    data_2 = data_2.sort_values(0)
    data = pd.merge(data_1, data_2, on=0)
    vals = data.values
    points = []
    indices = []
    for row in vals:
        coordinate = row[1:]
        indices.append(int(row[0]))
        point = Point(len(row)-1,coordinate)
        points.append(point)
    clusters = build_k_clusters(points,k,indices)
    print(clusters)
    while(iter_num<max_iter and Euclidean_Norm):
        old_norms = []
        for cluster in clusters:
            old_norms.append(cluster.norm)
        assign_cluster(points, clusters)
        updated_centorid(points, clusters)
        Euclidean_Norm=False
        for cluster in clusters:
            if(math.fabs(cluster.norm-old_norms[cluster.index])>epsilon):
                Euclidean_Norm=True
        iter_num+=1
    res = ""
    d = len(cluster.centroid_coordinates)
    for cluster in clusters:
        coordintes = cluster.centroid_coordinates
        for i in range(d):
            res += "{:.4f}".format(coordintes[i])
            if(i<d-1):
                res += ","
        res += "\n"
    print(res)

def build_k_clusters(points, k,indices):
    np.random.seed(0)
    clusters = []
    i = 1 
    n = len(points)
    indexs =np.array([i for i in range(n)])
    centroid_index = np.random.choice(indexs, size = 1)
    print(indices[centroid_index[0]])
    first_centroid = points[centroid_index[0]]
    # np_points = np.array(points, dtype=Point)
    # first_centroid = np.random.choice(np_points, size=1)
    clusters.append(Cluster(first_centroid.coordinates,0))
    d = np.array([distance(p.coordinates,first_centroid.coordinates) for p in points])
    pr = [0 for m in range(n)]
    while(i < k):
        for l in range(n):
            for j in range(i):
                dist = distance(points[l].coordinates,clusters[j].centroid_coordinates)
                if dist < d[l]:
                    d[l] = dist
        d_total = np.sum(d)
        for l in range(n):
            pr[l] = d[l]/d_total
        i += 1
        centroid_index = np.random.choice(indexs, p=pr, size = 1)
        print(indices[centroid_index[0]])
        clusters.append(Cluster(points[centroid_index[0]].coordinates,i-1))
        # centroid = np.random.choice(np_points, p=p, size=1)
        # clusters.append(Cluster(centroid[0].coordinates, i-1))
    return clusters    

def distance(coordinates_1, coordinates_2):
    dist = np.linalg.norm(coordinates_1-coordinates_2)
    return pow(dist,2)

def assign_cluster(points, clusters):
    k = len(clusters)
    num_points = len(points)
    for point in points:
        min = -1.0
        index = -1
        point_co = point.coordinates
        for cluster in clusters:
            cluster_co = cluster.centroid_coordinates
            distance = 0
            for i in range(point.d):
                distance += math.pow((point_co[i]-cluster_co[i]),2)
            distance = math.sqrt(distance)
            if(min<0 or distance<min):
                min=distance
                index=cluster.index
        point.set_clusterIndex(index)
        clusters[index].set_len(clusters[index].len+1)

def updated_centorid(points, clusters):
    points_by_clusters = [ [] for i in range(len(clusters))]
    for point in points:
        index=point.clusterIndex
        points_by_clusters[index].append(point)
    for cluster in clusters:
        centroid = [0 for i in range(point.d)]
        for point in points_by_clusters[cluster.index]:
            centroid = [x + y for x, y in zip(centroid, point.coordinates)]
        l = cluster.len
        centroid = [x/l for x in centroid]
        cluster.set_centroid(centroid)
        norm_values = [math.pow(x,2) for x in centroid]
        norm = sum(norm_values)
        norm = math.sqrt(norm)
        cluster.set_norm(norm)
        cluster.set_len(0)

def create_data_frames(file_name_1, file_name_2):
    data_1 = pd.read_csv(file_name_1, header=None)
    data_1 = data_1.sort_values(0)
    data_2 = pd.read_csv(file_name_2, header=None)
    data_2 = data_2.sort_values(0)
    data = pd.merge(data_1, data_2, on=0)
    vals = data.values
    #print(vals[0][2])
    points = []
    indices = []
    for row in vals:
        coordinate = row[1:]
        indices.append(row[0])
        point = Point(len(row)-1,coordinate)
        points.append(point)
    print([points[4]])

create_data_frames("input_1_db_1.txt","input_1_db_2.txt")
main()