# Mapper program to group the departments by the number of email communications with other departments and for each
# group count how many departments belong to the group

import sys

for d2d in sys.stdin:
    d2d = d2d.strip()
    dept1, dept2 = d2d.split(' ')
    if dept1 != dept2:
        print(f'{dept1} {dept2}')
