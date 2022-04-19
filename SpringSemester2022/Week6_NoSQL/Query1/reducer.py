"""
    Reducer program to calculate the number of emails sent by an employee
"""
import sys

curr_source_emp = None
curr_source_emp_cnt = 0
emp = None

'''
    Since, reducer receives data in sorted/ordered format. It counts emails for an employee
    until the employee id changes. It records the same for the employee and prints to the standard output.
'''
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
            print(f'{curr_source_emp}->{curr_source_emp_cnt}')
        curr_source_emp = s_emp
        curr_source_emp_cnt = 1

# Prints the last employee
if curr_source_emp == s_emp:
    print(f'{curr_source_emp}->{curr_source_emp_cnt}')


