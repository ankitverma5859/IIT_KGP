"""
    [[ This program assumes that mail self mail i.e 1 -> 1 as a distinct sender for receiver (1) ]]
    Mapper program to find the number of emails sent to the top 10 employees
"""

import sys

# s2r : Sender to Receiver
# Reads the data from the input file provided via command line, strips the whitespaces,
# splits on the space and prints on the standard output
for s2r_email_link in sys.stdin:
    s2r_email_link = s2r_email_link.strip()
    s_emp, r_emp = s2r_email_link.split()
    print(f'{s_emp}->{r_emp}')