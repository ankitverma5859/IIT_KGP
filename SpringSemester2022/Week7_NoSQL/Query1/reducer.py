# Reducer program to group the departments by the number of email communications with other departments and for each
# group count how many departments belong to the group

import sys

dict_dept_list = {}

# Creating an empty list for each department
for i in range(0, 42):
    dict_dept_list[i] = []

# Creating the department dictionary
for d2ld in sys.stdin:
    d2ld = d2ld.strip()
    dept, c_depts = d2ld.split(":")
    dept_list = c_depts.split(" ")

    #print(f'{dept} {dept_list}')
    len_depts = len(dept_list)

    for dept_id in dept_list:
        if dept_id not in dict_dept_list[int(dept)]:
            dict_dept_list[int(dept)].append(dept_id)

sorted_result = set()
result = {}
for key in dict_dept_list:
    len_dept = len(dict_dept_list[key])
    if len_dept > 0:
        #print(f'{key} {len_dept}')
        if len_dept not in result.keys():
            result[len_dept] = 1
        else:
            result[len_dept] += 1

for key in result:
    len_dept = len(result)
    if len_dept > 0:
        sorted_result.add(key)

# Printing the result
for item in sorted_result:
    print(f'{item} {result[item]}')


