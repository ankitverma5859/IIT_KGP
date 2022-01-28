# Task: Client should be abe to read and edit a file which is available on the server.

## Commands to EXECUTE:

## Host the server
```
./server <PORT>
```
  
## Run the client:
```
./client <SERVER_IP> <SERVER_PORT>
```
  
## Supported Commands:
```NLINEX``` : Gets the number of lines/entries in the server file.

```READX <k>``` : Gets the content at kth index.

```INSERTX <k> <message>``` : Inserts <message> at kth index.
  
```PRINTX```: Prints all the lines of file on console.
  
```CLOSE``` : Closes the connection.
  
***k*** : Index supports both forward and backwark indexing.
  
Forward Indexing: 0,1,...,(N-1), N
  
Backward Indexing: -N,-(N-1),...-2,-1
  

NOTE
  
**server_file.txt** is available on the server side with some data in ```<INDEX>  <MOVIE_NAME>```

