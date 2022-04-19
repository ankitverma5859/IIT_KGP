#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

char masterfile[] = "server_file.txt";
char backfile[] = "backup.txt";
const int MAX_INT = 32766;
const int BUFFER_SIZE = 255;
char line_count[10];

void error(char *msg);
void signal_handler(int sig);
void preprocess_file();

/*
    Function to check for valid commands
*/
int is_close(char* buffer);
int is_printx(char* buffer);
int is_nlinex(char* buffer);
int is_readx(char* buffer);
int is_insertx(char* buffer);

/*
    Helper functions to count lines, display content, and copy backup to master file
*/
char* count_lines_in_file();
void display_file_content();
void copy_backup_to_master();

/*
    Functions for READX Operations
*/
int is_valid_index(char* index);
char* read_from_file(int index);
char* readx(char* buffer);

/*
    Functions for INSERTX Operations
*/
void append_file(char* message);
void insert_at_top(char* message);
void insert_at_index(int index, char* message);
char* write_to_file(int index, char* message);
char* insertx(char* buffer);

/*
    Main Function
*/
int main(int argc, const char * argv[])
{
    signal(SIGINT, signal_handler);
    
    /*
     sock_fd and newsock_fd are file descriptors,
     i.e. array subscripts into the file descriptor table.
     These two variables store the values returned by the socket system
     call and the accept system call
     
     client_len stores the size of the address of the client. This is
     needed for the accept system call.
     
     port_no stores the port number on which the server accepts connections.
     
     n is the return value for the read() and write() calls; i.e. it
     contains the number of characters read or written
    */
    int sock_fd, newsock_fd, port_no, n;
    
    /*
     client_len stores the size of the address of the client. This is
     needed for the accept system call.
    */
    socklen_t client_len;
    
    /*
     The server reads characters from the socket connection into this buffer.
    */
    char buffer[256];
    
    /*
     sockaddr_in is a structure containing an internet address.
    */
    struct sockaddr_in server_addr, client_addr;
    
    if (argc < 2)
    {
        printf("ERROR: PORT not provided!\n");
        exit(1);
    }
    
    /*
     sockfd(ADDRESS_DOMAIN, TYPE_OF_SOCEKT, PROTOCOL)
     ADDRESS_DOMAIN = AF_UNIX, AF_INET(IPv4)
     TYPE_OF_SOCKET:
        SOCK_STREAM: A stream socket in which characters are read in continuous stream as if from a file or pipe
        SOCK_DGRAM: In a datagram socket, messages are read in chunks.
     PROTOCOL: 0
        Operating System chooses TCP for stream sockets and UDP for datagram sockets.
     
     socket system call return an entry in to the file descriptor. This value is used in all subsequent references to this socket. If the socket call fails, it returns -1.
    */
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sock_fd < 0)
    {
        error("ERROR: FAILED to open socket.");
    }
    
    /*
     bzero sets all the values in buffer to zero.
     bzero(POINTER_TO_BUFFER, SIZE_OF_BUFFER)
     */
    bzero((char *) &server_addr, sizeof(server_addr));
    port_no = atoi(argv[1]);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_no);
    
    /*
     bind() system call binds a socket to an address.
     It takes 3 arguments:
        Arg1: Socket File Descriptor
        Arg2: Address to which is bound
              It is a pointer to structure of type sockaddr, but what is
              passed is in sockaddr_in, and so it must be cast to correct
              type.
        Arg3: Size of the address to which it is bound
    */
    if(bind(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        error("ERROR: FAILED to bind.");
    }
    
    /*
     The listen() system call allows process to listen on socket for
     connections. It takes two arguments.
        Arg1: Socket File Descriptor
        Arg2: Size of the backlog queue i.e the number of connections that
              can be waiting while the process is handling a particular
              connection.
       
    */
    listen(sock_fd, 5);
    
    /*
     accept() call causes the process to block until the client connects
     to the server. Thus, it wakes up the process when a connection from a
     client has been successfully established.
     
        Arg1: Server file descriptor
        Arg2: Reference pointer to the address of the client on the other end
              of the connection.
        Arg3: Size of the structure.
     
     It returns a new file descriptor and all communicaition on
     this connection should be done on this file descriptor.
    */
    client_len = sizeof(client_addr);
    newsock_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &client_len);
    
    if (newsock_fd < 0){
        error("ERROR: FAILED to accept.");
    }
    
    preprocess_file();
    
    while (1)
    {
        bzero(buffer, 256);
        n = read(newsock_fd, buffer, 255);

        if(n < 0)
        {
            error("ERROR: FAILED to read from the socket.");
        }
        
        if(is_close(buffer))
        {
            printf("Connection Closed.\n");
            n = write(newsock_fd, "Close", 5);
            if(n < 0)
            {
                error("ERROR: FAILED to write to the socket.");
            }
            break;
        }
        else
        {
            char msg_for_client[256];
            
            if(is_nlinex(buffer))
            {
                printf("CLIENT issued NLINEX command.\n");
                strcpy(msg_for_client, "Number of lines in file: ");
                strcat(msg_for_client, count_lines_in_file());
            }
            else if (is_readx(buffer))
            {
                printf("CLIENT issued for READX command.\n");
                strcpy(msg_for_client, readx(buffer));
            }
            else if(is_insertx(buffer))
            {
                printf("CLIENT issued for INSERTX command.\n");
                strcpy(msg_for_client, insertx(buffer));
            }
            else if(is_printx(buffer))
            {
                printf("CLIENT issued for PRINTX command.\n");
                strcpy(msg_for_client, "PRINTX processed on server.");
                display_file_content();
            }
            else
            {
                printf("Invalid command issued from CLIENT.\n");
                strcpy(msg_for_client, "Invalid Command Issued.\nValid Commands:\n\tNLINEX\n\tREADX <k>\n\tINSERTX <k> <message>\n");
            }
            
            n = write(newsock_fd, msg_for_client, strlen(msg_for_client) + 1);
            
            if(n < 0)
            {
                error("ERROR: FAILED to write to the socket.");
            }
        }
    }
    
    close(newsock_fd);
    close(sock_fd);
    
    return 0;
}

/*
    Function to handle errors
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
    printf("\nServer shutting down...\n");
    exit(0);
}

/*
    Pre-processing the file since the file doenst contain \n for the last line
*/
void preprocess_file()
{
    FILE *fp;
    char new_line = '\n';
    fp = fopen(masterfile, "a");
    if (fp == NULL)
    {
        error("ERROR: FAILED to open master file.");
    }
    fprintf(fp, "%c", new_line);
    fclose(fp);
}

/*
    Function to check for the CLOSE/Close/close command
*/
int is_close(char* buffer)
{
    int to_close = 0;
    if (strncmp("CLOSE", buffer, 5) == 0 || strncmp("close", buffer,5) == 0 || strncmp("Close", buffer,5) == 0)
    {
        to_close = 1;
    }
    return to_close;
}

/*
    Function to check for PRINTX command
*/
int is_printx(char* buffer)
{
    int to_print = 0;
    if(strncmp("PRINTX", buffer, 6) == 0 || strncmp("printx", buffer, 6) == 0)
    {
        to_print = 1;
    }
    return to_print;
}

/*
    Function to check for NLINEX command
*/
int is_nlinex(char* buffer)
{
    int to_nlinex = 0;
    if((strncmp("NLINEX", buffer, 6) == 0 || strncmp("nlinex", buffer, 6) == 0) && buffer[6] == '\n')
    {
        to_nlinex = 1;
    }
    return to_nlinex;
}

/*
    Function to check for READX command
*/
int is_readx(char* buffer)
{
    int to_readx = 0;
    if((strncmp("READX", buffer, 5) == 0 || strncmp("readx", buffer, 5) == 0) && (buffer[5] == ' ' || buffer[5] == '\n'))
    {
        to_readx = 1;
    }
    return to_readx;
}

/*
    Function to check for INSERTX command
*/
int is_insertx(char* buffer)
{
    int to_insertx = 0;
    if((strncmp("INSERTX", buffer, 7) == 0 || strncmp("insertx", buffer, 7) == 0)  && (buffer[7] == ' ' || buffer[7] == '\n'))
    {
        to_insertx = 1;
    }
    return to_insertx;
}

/*
    Function to cound the number of lines in the master file
*/
char* count_lines_in_file()
{
    char line_buffer[BUFFER_SIZE];
    int number_of_lines = 0;
    
    FILE *fp;
    fp = fopen(masterfile, "r");
    
    if(fp == NULL)
    {
        error("ERROR: FAILED to read input file.");
    }
    
    while(fgets(line_buffer, BUFFER_SIZE, fp))
    {
        number_of_lines = number_of_lines + 1;
    }
    sprintf(line_count, "%d", number_of_lines);
    
    return line_count;
}

/*
    Function to display the contents of the file
*/
void display_file_content()
{
     char line_buffer[BUFFER_SIZE];
     
     FILE *fp;
     fp = fopen(masterfile, "r");
    
     if(fp == NULL)
     {
         error("ERROR: FAILED to read input file.");
     }
     
     while(fgets(line_buffer, BUFFER_SIZE, fp))
     {
         printf("%s",line_buffer);
     }
     printf("\n");
}

/*
    Function to copy the contents of the backup file to master file
*/
void copy_backup_to_master()
{
    FILE *bkp, *fp;
    fp = fopen(masterfile, "w");
    bkp = fopen(backfile,"r");
    
    if(fp == NULL)
    {
        error("ERROR: FAILED to open master file.");
    }
    
    if(bkp == NULL)
    {
        error("ERROR: FAILED to open backup file.");
    }
    
    fseek(fp, 0, SEEK_SET);
    fseek(bkp, 0, SEEK_SET);
    
    char c;
    while((c = getc(bkp)) != EOF)
    {
        putc(c, fp);
    }
    
    fclose(fp);
    fclose(bkp);
}

/*
    Function to check for a valid index.
*/
int is_valid_index(char* index)
{
    int index_len = strlen(index);
    
    for (int i = 0; i < index_len - 1; i++)
    {
        if (i == 0 && index[i] == '-')
        {
            continue;
        }
        
        if (!isdigit(index[i]))
        {
            return 0;
        }
    }
    return 1;
}

/*
    Function to read from the file
*/
char* read_from_file(int index)
{
    int num_of_lines = atoi(count_lines_in_file());
    char* line;
    
    if (index >= -(num_of_lines) && index < num_of_lines)
    {
        if (index < 0)
        {
            //Convert -index to positive index
            //-1 -> (num of lines - 1)      249
            //-2 -> (num of lines - 2)      248
            //
            //-250->(num of lines - 250)    0
            index = num_of_lines + index;
        }
        
        char line_buffer[BUFFER_SIZE];
        
        FILE *fp;
        fp = fopen(masterfile, "r");
        
        if(fp == NULL)
        {
            error("ERROR: FAILED to open input file.");
        }
        
        int i = 0;
        while(fgets(line_buffer, BUFFER_SIZE, fp))
        {
            if (i == index)
            {
                line = line_buffer;
                break;
            }
            i = i + 1;
        }
    }
    else
    {
        line = "ERROR: Index out of Range.\n";
    }
    
    return line;
}

/*
    Function to execute readx command
*/
char* readx(char* buffer)
{
    char* command;
    char* index;
    char* result;
    char* chars;
    
    command = strtok(buffer, " ");
    index = strtok(NULL, " ");
    
    if (index == NULL)
    {
        result = read_from_file(0);
    }
    else
    {
        chars = strtok(NULL, " ");
        if (chars == NULL)
        {
            if (is_valid_index(index))
            {
                result = read_from_file(atoi(index));
            }
            else
            {
                result = "INVALID Index issued.";
            }
            
        }
        else
        {
            result = "INVALID command format issued.\nVALID command format: READX <k>";
        }
    }
    return result;
}

/*
    Function to append the file
*/
void append_file(char* message)
{
    int total_lines = atoi(count_lines_in_file());
    FILE *fp;
    fp = fopen(masterfile, "a");
    
    if(fp == NULL)
    {
        error("ERROR: FAILED to open input file.");
    }
    
    fprintf(fp, "%d\t%s", total_lines + 1, message);
    fclose(fp);
}

/*
    Function to add content at the top of the file
*/
void insert_at_top(char* message)
{
    char *content;
    content = message;
    int index = 1;
    FILE *fp, *bkp_fp;
    
    fp = fopen(masterfile, "r");
    bkp_fp = fopen(backfile, "w");
    
    if (fp == NULL)
    {
        error("ERROR: FAILED to open master file file.");
    }
    
    if (bkp_fp == NULL)
    {
        error("ERROR: FAILED to open backup file.");
    }
    
    fprintf(bkp_fp, "%d\t%s", index, message);
    fseek(bkp_fp, 0, SEEK_END);
    
    char line[250];
    while(fscanf(fp, "%[^\n] ", line) != EOF)
    {
        char *lnum = strtok(line, "\t");
        char *movie = strtok(NULL, "");
        fprintf(bkp_fp, "%d\t%s\n", atoi(lnum) + 1, movie);
    }
    
    fclose(fp);
    fclose(bkp_fp);
    
    copy_backup_to_master();
}

/*
    Function to add at a particular index of the file
*/
void insert_at_index(int index, char* message)
{
    FILE *fp, *bkp_fp;
    char c;
    int newlines = 0;
    
    fp = fopen(masterfile, "r");
    bkp_fp = fopen(backfile, "w");
    
    if(fp == NULL)
    {
        error("ERROR: FAILED to open master file.");
    }
    
    if (bkp_fp == NULL)
    {
        error("ERROR: FAILED to open backup file.");
    }
    
    while(newlines < index)
    {
        c = getc(fp);
        if(c == '\n')
            newlines = newlines + 1;
        
        if (newlines < index)
        {
            putc(c, bkp_fp);
        }
    }
    putc('\n', bkp_fp);
    
    fprintf(bkp_fp, "%d\t%s", newlines + 1, message);
    char line[250];
    
    while(fscanf(fp, "%[^\n] ", line) != EOF)
    {
        char *lnum = strtok(line, "\t");
        char *movie = strtok(NULL, "");
        fprintf(bkp_fp, "%d\t%s\n", atoi(lnum) + 1, movie);
    }
    
    fclose(bkp_fp);
    fclose(fp);
    
    copy_backup_to_master();
}

/*
    Function to write to the file
*/
char* write_to_file(int index, char* message)
{
    char* output;
    char buffer[256];
    
    if (index == MAX_INT)
    {
        append_file(message);
        output = "INSERTX sucessfull\n";
    }
    else
    {
        int num_of_lines = atoi(count_lines_in_file());
        
        if (index >= -(num_of_lines) && index <= num_of_lines)
        {
            if (index < 0)
            {
                //Convert -index to positive index
                //-1 -> (num of lines - 1)      249
                //-2 -> (num of lines - 2)      248
                //
                //-250->(num of lines - 250)    0 first line
                index = num_of_lines + index;
            }
            
            if (index == 0) //add at the top
            {
                insert_at_top(message);
                output = "INSERTX sucessfull at the top.\n";
            }
            else if (index == num_of_lines) // append at the end
            {
                append_file(message);
                output = "INSERTX sucessfull at the end.\n";
            }
            else // add in the middle
            {
                insert_at_index(index, message);
                output = "INSERTX sucessfull at the index.\n";
            }
        }
        else
        {
            output = "ERROR: Index out of Range.\n";
        }
    }
    return output;
}

/*
    Function to execute insertx command
*/
char* insertx(char* buffer)
{
    char* bufferDup = strdup(buffer);
    char* command;
    char* index;
    char* message;
    char* result;
    
    command = strtok(buffer, " "); // Split INSERTX and rest of the text
    index = strtok(NULL, " "); // Splits the first word from the remaining text of above split
    if(index == NULL)
    {
        result = "INVALID command format issued.\nVALID command format: INSERTX <k> <message>";
    }
    else
    {
        if (is_valid_index(index))
        {
            message = strtok(NULL, "");
            if (message == NULL)
            {
                result = "INVALID command format issued.\nVALID command format: INSERTX <k> <message>";
            }
            else
            {
                result = write_to_file(atoi(index), message);
            }
        }
        else
        {
            //case when k is not given as an input
            command = strtok(bufferDup, " ");
            message = strtok(NULL, "");
            result = write_to_file(MAX_INT, message);
            free(bufferDup);
        }
    }
    return result;
}
