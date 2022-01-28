#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(char *msg);
void signal_handler(int sig);

/*
    Main function
*/
int main(int argc, const char * argv[])
{
    /*
        To handle the signal CTRL + C
    */
    signal(SIGINT, signal_handler);
    
    int sock_fd, port_no, n;
    
    /*
     The variable server_addr will contain the address of the server to
     to which client wants to connect.
     */
    struct sockaddr_in server_addr;
    
    /*
     The variable server is a pointer to structure of type hostent.
     */
    struct hostent *server;
    
    char buffer[256];
    
    if (argc < 3)
    {
        printf("ERROR: SERVER_IP or SERVER_PORT or both not provided!\n");
        exit(1);
    }
    
    /*
     Argument 1 is treated as the Server IP
     Argument 2 is treated as the Server Port
    */
    server = gethostbyname(argv[1]);
    port_no = atoi(argv[2]);
    
    if(server == NULL)
    {
        error("ERROR: No such host found.");
    }
    
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(sock_fd < 0)
    {
        error("ERROR: FAILED to open socket.");
    }
    
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port_no);
    
    if(connect(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        error("ERROR: FAILED to connect.");
    }
    
    while(1)
    {
        bzero(buffer, 256);
        printf("\nEnter your COMMAND:");
        fgets(buffer, 255, stdin);
        
        n = write(sock_fd, buffer, strlen(buffer));
        
        if(n < 0)
        {
            error("ERROR: FAILED to write to the socket.");
        }
        
        bzero(buffer,256);
        
        n = read(sock_fd, buffer, 255);
        
        if (n < 0)
        {
            error("ERROR: FAILED to read from socket.");
        }
        
        if (strncmp("Close", buffer, 5) == 0)
        {
            printf("Connection Closed.\n");
            break;
        }
        else
        {
            printf("%s\n",buffer);
        }
    }
    close(sock_fd);
    return 0;
 }

/*
    Function to print an error
*/
void error(char *msg)
{
    perror(msg);
    exit(1);
}

/*
    Function to handle CTRL + C signal
*/
void signal_handler(int sig)
{
    signal(sig, SIG_IGN);
    printf("\nClient shutting down...\n");
    exit(0);
}

