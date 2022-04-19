# Reducer program to find out how many common departments each pair of nodes have in the event network created in the
# last query
import sys
import numpy as np

hash_vals = {}
rows = 42
cols = 42
d2d_adj = np.zeros([rows, cols], dtype=int)

# Creating the adjacency matrix
for d2d in sys.stdin:
    d2d = d2d.strip()
    d1, d2 = d2d.split(" ")
    d1 = int(d1)
    d2 = int(d2)
    d2d_adj[d1][d2] = 1
    d2d_adj[d2][d1] = 1
    #print(f'{d1} {d2}')

res = np.matmul(d2d_adj, d2d_adj)
#print(d2d_adj)
#print("Res")
#print(res)

# Printing the result
for row in range(rows):
    for col in range(cols):
        if row != col and res[row][col] != 0:
            if row < col:
                identifier = str(row) + '_' + str(col)
            else:
                identifier = str(col) + '_' + str(row)
            if identifier not in hash_vals:
                print(f'{row}, {col}, #{res[row][col]}')
                hash_vals[identifier] = 0
