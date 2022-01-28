#Task: Client should be abe to read and edit a file which is available on the server.
### server_file.txt is available on the server side with some data in <INDEX>\t<MOVIE_NAME> format.

##Commands to EXECUTE:

##Host the server
```
./server <PORT>
```
  
##Run the client:
```
./client <SERVER_IP> <SERVER_PORT>
```
  
  ##Supported Commands:
  NLINEX : Gets the number of lines/entries in the server file.
  READX <k> : Gets the content at kth index.
  INSERTX <k> <message> : Inserts <message> at kth index.
  CLOSE : Closes the connection.

