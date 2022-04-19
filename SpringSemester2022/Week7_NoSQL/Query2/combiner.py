# Combiner program to find the strongly connected nodes
import sys

# Creating the distinct pairs to feed the reducer as an input
dict_pairs = {}
for d2d in sys.stdin:
    d2d = d2d.strip()
    lv, rv = d2d.split(" ")

    #hash_val = hash(lv) + hash(rv)
    hash_val = lv + "_" + rv
    if hash_val not in dict_pairs.keys():
        dict_pairs[hash_val] = [lv, rv, 1]
    else:
        val = dict_pairs[hash_val]
        val[2] += 1
        dict_pairs[hash_val] = val

for key in dict_pairs:
    val = dict_pairs[key]
    print(f'{key}:{val[0]} {val[1]} {val[2]}')
