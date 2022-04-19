"""
    Mapper program to the most influential department in terms of outgoing emails
"""

# Loading the input files
send2recv = open('../network.txt', 'r')
d_labels = open('../dept_labels.txt', 'r')

# Printing the sender to receiver email links on the standard output
for line in send2recv:
    line = line.strip()
    sender, receiver = line.split()
    print(f'{sender}->{receiver}->S2R')

# Printing the employee to department id relationship on the standard output
for line in d_labels:
    line = line.strip()
    employee, department = line.split()
    print(f'{employee}->{department}->E2D')