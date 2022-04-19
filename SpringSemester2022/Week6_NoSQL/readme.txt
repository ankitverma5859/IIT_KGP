File Structure:

A6_21CS60A04
    /Query1
        Makefile
        mapper.py
        reducer.py
        result.txt
    /Query2
        Makefile
        mapper.py
        reducer.py
        result.txt
    /Query3
        Makefile
        mapper.py
        reducer.py
        result.txt
    /Query4
        Makefile
        mapper.py
        reducer.py
        result.txt
    dept_labels.txt
    network.txt
    readme.txt

Query 1: Program to calculate the number of emails sent by an employee
        How to execute?
        cd /Query1
        make run
        result.txt is stored in the same directory i.t /Query1

        Result format:
        Employee Id -> Number of emails sent
        0->41
        1->1
        2->84
        3->56
        ...

Query 2: Program to find the top 10 employees in terms of outgoing emails
        How to execute?
        #This program is dependent upon the output of Task1. Thus, result.txt of Task1 must be present in its directory.
        cd /Query2
        make run
        result.txt is stored in the same directory i.t /Query2

        Result format:
        Rank		Employee Id
        Rank 1		160
        Rank 2		82
        Rank 3		121
        Rank 4		107
        ...

Query 3: Program to find the number of emails sent to the top 10 employees
        How to execute?
        cd /Query3
        make run
        result.txt is stored in the same directory i.t /Query3

        Result format:
        Employee Id(Top 10)		Number of Employees sent email
        160							212
        82							121
        121							157
        107							169
        ...

Query 4: Program to the most influential department in terms of outgoing emails
        How to execute?
        cd /Query4
        make run
        result.txt is stored in the same directory i.t /Query4

        Result format:
        Department	Number of Outgoing Emails
        36			2110