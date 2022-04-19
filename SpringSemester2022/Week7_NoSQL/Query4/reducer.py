# Reducer program to find the shortest path to all the minimum degree nodes from the maximum one and the corresponding
# shortest path length
import sys
import numpy as np
#import networkx as nx
#import matplotlib.pyplot as plt

# Uncomment for graph visualization
#G = nx.DiGraph()


rows = 42
cols = 42
dict_pairs = {}
d2d_adj = np.zeros([rows, cols], dtype=int)
d2d_weight = np.zeros([rows, cols], dtype=int)
d_ecounts = {}


# Helper method to find the shortest distance from to node to other nodes
def find_shortest_distance(graph, start_index):
    distances = [float("inf") for _ in range(len(graph))]
    visited = [False for _ in range(len(graph))]
    distances[start_index] = 0

    while 1:
        short_dist = 999999
        short_index = -1
        for i in range(len(graph)):
            if distances[i] < short_dist and not visited[i]:
                short_dist = distances[i]
                short_index = i

        if short_index == -1:
            return distances

        for i in range(len(graph[short_index])):
            if graph[short_index][i] != 0 and distances[i] > distances[short_index] + graph[short_index][i]:
                distances[i] = distances[short_index] + graph[short_index][i]

        visited[short_index] = True


# Creating the adjacency matrix from the values obtained from stdin
for d2d in sys.stdin:
    d2d = d2d.strip()
    d1, d2 = d2d.split(" ")
    d1 = int(d1)
    d2 = int(d2)

    d2d_weight[d1][d2] += 1
    d2d_weight[d2][d1] += 1

    hash_val = str(d1) + "_" + str(d2)

    if d1 < d2:
        hash_val = str(d1) + "_" + str(d2)
    else:
        hash_val = str(d2) + "_" + str(d1)

    if hash_val in dict_pairs.keys():
        1 == 1
    else:
        dict_pairs[hash_val] = 1
        d2d_adj[d1][d2] += 1
        d2d_adj[d2][d1] += 1

# Code to print the adjacency matrix of the graph
'''
for i in range(rows):
    for j in range(cols):
        print(d2d_adj[i][j], end=" ")
    print('\n')
'''

for row in range(rows):
    count = 0
    for col in range(cols):
        count += d2d_adj[row][col]
    d_ecounts[row] = count

# Finding the maximum and minimum degree among the nodes
max = -1
min = 100000
for key in d_ecounts:
    val = d_ecounts[key]
    if val > max:
        max = val

    if val < min:
        min = val


max_nodes = []
min_nodes = []

# Creating the list of Max(maximum degree) and Min(minimum degree) nodes
for key in d_ecounts:
    val = d_ecounts[key]
    if val == max:
        max_nodes.append(key)
    elif val == min:
        min_nodes.append(key)

#print(f'Max Nodes: {max_nodes}')
#print(f'Min Nodes: {min_nodes}')

# Uncomment for graph visualization
'''
for i in range(rows):
    for j in range(cols):
        if d2d_adj[i][j] > 0:
            G.add_edge(i, j)


nx.draw(G, with_labels=True)
plt.show()
'''

shortest_path_length = find_shortest_distance(d2d_weight, max_nodes[0])

# Display the Max(maximum degree) Min(minimum degree) Length of Path
for max_node in max_nodes:
    for min_node in min_nodes:
        print(f'{max_node} {min_node} {shortest_path_length[min_node]}')



