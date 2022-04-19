Dependencies:
sys
numpy

File Structure:

A6_21CS60A04
    /dataset
        event1.txt
        ...
        event10.txt
    /Query1
        Makefile
        mapper.py
        reducer.py
        combiner.py
        result_event.txt
    /Query2
        Makefile
        mapper.py
        reducer.py
        combiner.py
        result_strong.txt
    /Query3
        Makefile
        mapper.py
        reducer.py
        result_common.txt
    /Query4
        Makefile
        mapper.py
        reducer.py
        result_shortest.txt
        NetworkGraph.png            (Visualization of the Network Created from the nodes)
    readme.txt
    Makefile

How to execute all the routines?
cd A7_21CS60A04
make run

Query 1: Program to group the departments by the number of email communications with other departments and for each
        group count how many departments belong to the group
        How to execute?
        cd /Query1
        make run
        result_event.txt is stored in the same directory i.t /Query1

        Result format:
        Department Id Number of Departments
        1 1
        5 1
        9 1
        16 1
        18 1
        19 2
        ...

Query 2: Program to find the strongly connected nodes
        How to execute?
        cd /Query2
        make run
        result_strong.txt is stored in the same directory i.t /Query2

        Result format:
        Dept Id		Dept Id
        0 1
        0 36
        0 4
        0 7
        1 14
        1 15
        1 23
        ...

Query 3: Program to find out how many common departments each pair of nodes have in the event network created in the
        last question
        Note: This query is dependent upon the result of Query 2
        How to execute?
        cd /Query3
        make run
        result_common.txt is stored in the same directory i.t /Query3

        Result format:
        Node1, Node2, #CommonDepartments
        0, 1, #3
        0, 10, #2
        0, 11, #2
        0, 13, #2
        0, 14, #3
        ...

Query 4: Program to find the shortest path to all the minimum degree nodes from the maximum one and the corresponding
         shortest path length
        How to execute?
        cd /Query4
        make run
        result_shortest.txt is stored in the same directory i.t /Query4

        Result format:
        NodeWithMaxDegree, NodeWithMinDegree, pathLength
        36 33 5
        36 41 3

        For graphical visualization refer to NetworkGraph.png saved in the same directory