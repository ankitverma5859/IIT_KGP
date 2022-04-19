# Mapper program to find out how many common departments each pair of nodes have in the event network created in the
# last query
import sys

# Splitting he input data to feed the reducer
for d2d in sys.stdin:
    d2d = d2d.strip()
    dept1, dept2 = d2d.split(' ')
    print(f'{dept1} {dept2}')
