"""
    Mapper program to the most influential department in terms of outgoing emails
"""

import sys


d2e = {}
d2e_counts = {}
s2r_links = []


# Function to return the department id of an employee
def find_department(emp):
    for key in d2e:
        if emp in d2e[key]:
            return key


# Creating an empty dict of the departments
for i in range(0, 42):
    d2e[i] = []
    d2e_counts[i] = 0

# Finding the employee id's of the departments
for line in sys.stdin:
    line = line.strip()
    t1, t2, t3 = line.split('->')

    if t3 == "S2R":
        s2r_link = t1 + " " + t2
        s2r_links.append(s2r_link)
    elif t3 == "E2D":
        dept = int(t2)
        d2e[dept].append(t1)

# Finding the number of outgoing emails of the departments
for s2r_link in s2r_links:
    sender, receiver = s2r_link.split()
    s_dept = find_department(sender)
    r_dept = find_department(receiver)
    if s_dept != r_dept:
        d2e_counts[s_dept] += 1

# Finding the most influential department in terms of number of outgoing emails
email_counts = 0
most_influential_dept = -1
for dept in d2e_counts:
    if d2e_counts[dept] > email_counts:
        email_counts = d2e_counts[dept]
        most_influential_dept = dept

print(f'Department\tNumber of Outgoing Emails')
print(f'{most_influential_dept}\t\t\t{email_counts}')




