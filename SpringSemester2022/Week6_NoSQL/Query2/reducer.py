"""
    Reducer program to find the top 10 employees in terms of outgoing emails
"""

import sys

curr_source_emp = None
curr_source_emp_cnt = 0
emp_to_email_cnts = {}
emp = None

for s2r_email_link in sys.stdin:
    s2r_email_link = s2r_email_link.strip()
    # s_emp : Sender Employee
    # e_emp : Receiver Employee
    s_emp, r_emp = s2r_email_link.split('->')

    if curr_source_emp == s_emp:
        # Incrementing the count of email for the employee
        curr_source_emp_cnt += 1
    else:
        if curr_source_emp:
            emp_to_email_cnts[curr_source_emp] = curr_source_emp_cnt
        curr_source_emp = s_emp
        curr_source_emp_cnt = 1

# Prints the last employee
if curr_source_emp == s_emp:
    emp_to_email_cnts[curr_source_emp] = curr_source_emp_cnt


# Prints the top 10 employees in terms of number of outgoing emails sent
print(f'Rank\t\tEmployee Id')
for itr in range(0, 10):
    max_val = 0
    for key, value in emp_to_email_cnts.items():
        if max_val < value:
            max_val = value
            emp_id = key
    print(f'Rank {itr+1}\t\t{emp_id}')
    emp_to_email_cnts.pop(emp_id)

'''
sorted_cnts = sorted(emp_to_email_cnts.items(), key=lambda x: x[1])

for itr in reversed(range(-10, 0)):
    top_10.append(sorted_cnts[itr][0])

print(f'Rank\t\tEmployee Id')
for itr, item in enumerate(top_10):
    print(f'Rank {itr+1}\t\t{item}')
'''

