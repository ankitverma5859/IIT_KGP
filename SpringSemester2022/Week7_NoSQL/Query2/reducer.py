# Reducer program to find the strongly connected nodes
import sys

dict_pairs = {}

for d_val in sys.stdin:
    d_val = d_val.strip()
    hash_val, dept = d_val.split(":")
    dept1, dept2, count = dept.split(" ")
    #print(f'{hash_val} {dept1} {dept2} {count}')
    if hash_val not in dict_pairs.keys():
        dict_pairs[hash_val] = [dept1, dept2, count]
    else:
        val = dict_pairs[hash_val]
        val[2] = int(val[2]) + int(count)
        dict_pairs[hash_val] = val

# Printing the strongly connected nodes
#print(dict_pairs)
dict_key_counts = {}
for key in dict_pairs:
    val = dict_pairs[key]
    v0 = int(val[0])
    v1 = int(val[1])
    v2 = int(val[2])
    if v2 > 50:
        if v0 > v1:
            print(f'{v1} {v0}')
        else:
            print(f'{v0} {v1}')
