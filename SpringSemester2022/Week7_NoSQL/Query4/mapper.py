# Mapper program to find the shortest path to all the minimum degree nodes from the maximum one and the corresponding
# shortest path length
import sys

# Splitting the data to feed the reducer program
for d2d in sys.stdin:
    d2d = d2d.strip()
    dept1, dept2 = d2d.split(' ')
    if dept1 != dept2:
        print(f'{dept1} {dept2}')