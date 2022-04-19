# Mapper program to find the strongly connected nodes
import sys

for d2d in sys.stdin:
    d2d = d2d.strip()
    dept1, dept2 = d2d.split(' ')
    if dept1 != dept2:
        if dept1 < dept2:
            print(f'{dept1} {dept2}')
        else:
            print(f'{dept2} {dept1}')
