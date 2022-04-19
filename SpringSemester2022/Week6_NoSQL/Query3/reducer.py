"""
    [[ This program assumes that mail self mail i.e 1 -> 1 as a distinct sender for receiver (1) ]]
    Reducer program to find the number of emails sent to the top 10 employees
"""

import sys

# Loading the result of Task2
top10_empid = {}
file = open('../Query2/result.txt', 'r')
top10_emp = file.readlines()

# Creating a dict of top10 employee ids
for itr, emp in enumerate(top10_emp):
    if itr > 0:
        emp = emp.strip()
        rank, emp_id = emp.split('\t\t')
        top10_empid[emp_id] = []

top_10 = list(top10_empid.keys())

# Finding the list of employees who sent email to the top10 employees
for line in sys.stdin:
    line = line.strip()
    s_emp, r_emp = line.split('->')
    if r_emp in top_10:
        if s_emp not in top10_empid[r_emp]:
            top10_empid[r_emp].append(s_emp)

print(f'Employee Id(Top 10)\t\tNumber of Employees sent email')
for key in top10_empid:
    print(f'{key}\t\t\t\t\t\t\t{len(top10_empid[key])}')
