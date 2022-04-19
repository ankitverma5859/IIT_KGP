# Combiner program to group the departments by the number of email communications with other departments and for each
# group count how many departments belong to the group

import sys

hashes = []
dict_dept_list = {}

# Creating an empty list for each department
for i in range(0, 42):
    dict_dept_list[i] = []

# Creating the list of distinct departments
for d2d in sys.stdin:
    d2d = d2d.strip()
    dept1, dept2 = d2d.split(' ')
    if dept1 != dept2:
        lr = str(dept1) + str(dept2)
        rl = str(dept2) + str(dept1)
        hash_val = hash(lr) + hash(rl)
        if hash_val not in hashes:
            hashes.append(hash_val)
            dict_dept_list[int(dept1)].append(dept2)

            dept_id2 = dict_dept_list[int(dept2)]
            dict_dept_list[int(dept2)].append(dept1)

# Creating the input for the reducer
for key in dict_dept_list:
    len_dept = len(dict_dept_list[key])
    if len_dept > 0:
        print(key, end=":")
        dept_list = dict_dept_list[key]
        for itr in range(0, len_dept):
            print(dept_list[itr], end=" ")
        print("")
