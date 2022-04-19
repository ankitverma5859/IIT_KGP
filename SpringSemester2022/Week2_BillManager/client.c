//
//  client.c
//  W2_BillManager
//
//  Created by Ankit Verma on 20/01/22.
//

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<regex.h>
#include <sys/stat.h>

const int BUFFER_SIZE = 256;

char *REG_EXIT = "^/exit\n$";
char *REG_SORT = "^/sort [a-zA-Z0-9_]*.txt (date|item_name|price)\n$";
char *REG_MERGE = "^/merge [a-zA-Z0-9_]*.txt [a-zA-Z0-9_]*.txt [a-zA-Z0-9_]*.txt (date|item_name|price)\n$";
char *REG_SIMILARITY = "^/similarity [a-zA-Z0-9_]*.txt [a-zA-Z0-9_]*.txt\n$";

char *EXIT = "exit";
char *SORT = "sort";
char *MERGE = "merge";
char *SIMILARITY = "similarity";
char *NONE = "none";
char *BASE_LOCATION = "data/";
char *FILESIZE = "filesize";
char *SENDFILE = "sendfile";
char *SORTED_FILE = "data/sortedfile.txt";

void error(char *err);
void signal_handler(int sig);
int is_exit(char* buffer);
void preprocess(char *filename);
int regex_match(char *string, char *regex_string);
char* validate_command(char *buffer);
int validate_inputfiles(char *command);
long long stat_filesize(char *filename);
void write_error_check(int n);
void send_file(char* file_1, long long filesize, int sock_fd);
long convert_string_to_long(char *size);
long long calculate_filesize(char* filename);
/*
    Main function
*/
int main(int argc, const char * argv[])
{
    /*
        To handle the signal CTRL + C
    */
    signal(SIGINT, signal_handler);
   
    /*
     sock_fd is a file descriptors
    */
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
    
    char buffer[BUFFER_SIZE];
    
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
        start:
        bzero(buffer, BUFFER_SIZE);
        printf("\nEnter your COMMAND:");
        fgets(buffer, BUFFER_SIZE, stdin);
        
        if(strcmp(validate_command(buffer), NONE) != 0)
        {
            char bkp_command[BUFFER_SIZE];
            strcpy(bkp_command, buffer);
            
            char* command = validate_command(buffer);
            if(strcmp(command, EXIT) == 0)
            {
                n = write(sock_fd, buffer, strlen(buffer));
                write_error_check(n);
                
                close(sock_fd);
                error("Connection closed.");
            }
            
            if(strcmp(command, SORT) == 0)
            {
                //STEP 1: SEND sort command to server
                n = write(sock_fd, buffer, strlen(buffer));
                write_error_check(n);
                
                //STEP 2: CHECK server response for "filesize"
                bzero(buffer, BUFFER_SIZE);
                n = read(sock_fd, buffer, BUFFER_SIZE);
                
                if (strncmp(FILESIZE, buffer, 8) == 0)
                {
                    //STEP 3: Create filename
                    char file_1[100];
                    char *filename_1;
                    strtok(bkp_command, " ");
                    filename_1 = strtok(NULL, " ");
                    
                    
                    strcpy(file_1, BASE_LOCATION);
                    strcat(file_1, filename_1);
                    
                    //STEP : Calculate filesize
                    long long filesize = stat_filesize(file_1);
                    //printf("Filesize: %lld\n", filesize);
                    
                    //STEP : Send filesize to server
                    char fsize[BUFFER_SIZE];
                    sprintf(fsize, "%lld", filesize);
                    n = write(sock_fd, fsize, strlen(fsize));
                    
                    //STEP: Wait for server's message to send file
                    bzero(buffer,BUFFER_SIZE);
                    n = read(sock_fd, buffer, BUFFER_SIZE);
                    if (strncmp(SENDFILE, buffer, 8) == 0)
                    {
                        //Send unsorted file
                        send_file(file_1, filesize, sock_fd);
                        
                        //Receive filesize of the sorted file
                        bzero(buffer, BUFFER_SIZE);
                        n = read(sock_fd, buffer, BUFFER_SIZE);
                        if(strncmp(buffer, "INVALID_FILE_CONTENT", 20) == 0)
                        {
                            printf("Invalid file content.\n");
                        }
                        else
                        {
                            long filesize = convert_string_to_long(buffer);
                            
                            //Request for sorted file content
                            n = write(sock_fd, SENDFILE, strlen(SENDFILE));
                            
                            //Read the content of sorted file content
                            printf("File: %s\n", file_1);
                            char data[filesize];
                            n = read(sock_fd, data, filesize+1);
                            FILE *fp;
                            fp = fopen(file_1, "w");
                            printf("After Sorting:\n");
                            for(int i = 0; data[i] != '\0'; i++)
                            {
                                printf("%c", data[i]);
                                fputc(data[i], fp);
                            }
                            printf("Sorted data is stored in %s file.\n", file_1);
                            fclose(fp);
                            
                        }
                    }
                    bzero(buffer,BUFFER_SIZE);
                    
                }
            }
            else if(strcmp(command, MERGE) == 0)
            {
                //S1: Send merge command to server
                n = write(sock_fd, buffer, strlen(buffer));
                bzero(buffer,BUFFER_SIZE);
                
                //S4: CHECK server response for "filesize"
                bzero(buffer, BUFFER_SIZE);
                n = read(sock_fd, buffer, BUFFER_SIZE);
                if (strncmp(FILESIZE, buffer, 8) == 0)
                {
                    char *filename_1;
                    char *filename_2;
                    char *filename_3;
                    char file_1[100];
                    char file_2[100];
                    strtok(bkp_command, " ");
                    filename_1 = strtok(NULL, " ");
                    filename_2 = strtok(NULL, " ");
                    filename_3 = strtok(NULL, "");
                    strcpy(file_1, BASE_LOCATION);
                    strcat(file_1, filename_1);
                    strcpy(file_2, BASE_LOCATION);
                    strcat(file_2, filename_2);
                    
                    printf("File1: %s\n",file_1);
                    printf("File2: %s\n",file_2);
                    
                    
                    long long filesize_1 = stat_filesize(file_1);
                    long long filesize_2 = stat_filesize(file_2);
                    printf("FileSize1: %lld\n",filesize_1);
                    printf("FileSize2: %lld\n",filesize_2);
                    
                    // S5: calculate and send size of file 1
                    char fsize_1[BUFFER_SIZE];
                    sprintf(fsize_1, "%lld", filesize_1);
                    n = write(sock_fd, fsize_1, strlen(fsize_1));
                    
                    // S8 check if server asked for file and then send
                    bzero(buffer,BUFFER_SIZE);
                    n = read(sock_fd, buffer, BUFFER_SIZE);
                    if (strncmp(SENDFILE, buffer, 8) == 0)
                    {
                        send_file(file_1, filesize_1, sock_fd);
                    }
                    
                    
                    bzero(buffer, BUFFER_SIZE);
                    n = read(sock_fd, buffer, BUFFER_SIZE);
                    if (strncmp(FILESIZE, buffer, 8) == 0)
                    {
                        // S12: calculate and send size of file 1
                        char fsize_2[BUFFER_SIZE];
                        sprintf(fsize_2, "%lld", filesize_2);
                        n = write(sock_fd, fsize_2, strlen(fsize_2));
                        
                        // S15 check if server asked for file and then send
                        bzero(buffer,BUFFER_SIZE);
                        n = read(sock_fd, buffer, BUFFER_SIZE);
                        if (strncmp(SENDFILE, buffer, 8) == 0)
                        {
                            send_file(file_2, filesize_2, sock_fd);
                        }
                    }
                }
                n = read(sock_fd, buffer, BUFFER_SIZE);
                printf("Server Response: %s", buffer);
            }
            else if(strcmp(command, SIMILARITY) == 0)
            {
                // Send Similarity command to server
                n = write(sock_fd, buffer, strlen(buffer));
                bzero(buffer,BUFFER_SIZE);
                
                char file_1[BUFFER_SIZE], file_2[BUFFER_SIZE];
                char *filename_1;
                char *filename_2;
                strtok(bkp_command, " ");
                filename_1 = strtok(NULL, " ");
                filename_2 = strtok(NULL, " ");
                filename_2[strlen(filename_2) - 1] = '\0';
                
                strcpy(file_1, BASE_LOCATION);
                strcat(file_1, filename_1);
                
                strcpy(file_2, BASE_LOCATION);
                strcat(file_2, filename_2);
                
                //STEP : Calculate filesize
                long long filesize_1 = stat_filesize(file_1);
                long long filesize_2 = stat_filesize(file_2);
                
                //STEP : Send filesize to server
                char fsize[256];
                sprintf(fsize, "%lld", filesize_1);
                n = write(sock_fd, fsize, strlen(fsize));
                
                //STEP: Wait for server's message to send file
                bzero(buffer,BUFFER_SIZE);
                n = read(sock_fd, buffer, BUFFER_SIZE);
                if (strncmp(SENDFILE, buffer, 8) == 0)
                {
                    //Send unsorted file
                    send_file(file_1, filesize_1, sock_fd);
                    
                    //Receive filesize/invalid response of the sorted file
                    bzero(buffer, BUFFER_SIZE);
                    n = read(sock_fd, buffer, BUFFER_SIZE);
                    if(strncmp(buffer, "INVALID_FILE_CONTENT", 20) == 0)
                    {
                        printf("Invalid file content in %s.\n", filename_1);
                    }
                    else
                    {
                        //Send file2
                        //STEP : Send filesize to server
                        char fsize[256];
                        sprintf(fsize, "%lld", filesize_1);
                        n = write(sock_fd, fsize, strlen(fsize));
                        
                        //STEP: Wait for server's message to send file
                        bzero(buffer,BUFFER_SIZE);
                        n = read(sock_fd, buffer, BUFFER_SIZE);
                        if (strncmp(SENDFILE, buffer, 8) == 0)
                        {
                            //
                            send_file(file_2, filesize_2, sock_fd);
                            //Receive filesize/invalid response of the sorted file
                            bzero(buffer, BUFFER_SIZE);
                            n = read(sock_fd, buffer, BUFFER_SIZE);
                            printf("Final: %s", buffer);
                            if(strncmp(buffer, "INVALID_FILE_CONTENT", 20) == 0)
                            {
                                printf("Invalid file content in %s.\n", filename_1);
                            }
                            else
                            {
                                printf("Server is processing the request.\n");
                            }
                        }
                    }
                }
                
                
                bzero(buffer,BUFFER_SIZE);
                //n = read(sock_fd, buffer, BUFFER_SIZE);
                //printf("Server Response: %s", buffer);
            }
            
            
        }
        else
        {
            printf("Invalid Command.\nValid Commands:\n\t/sort <filename> <field>\n\t/merge <filename> <filename> <filename> <field>\n\t/similarity <filename> <filename>\n\t/exit\n");
        }
    }

    close(sock_fd);
    return 0;
}

long long calculate_filesize(char* filename)
{
    char file_1[100];
    strcpy(file_1, BASE_LOCATION);
    strcat(file_1, filename);
    
    //STEP : Calculate filesize
    return stat_filesize(file_1);
}

void send_file(char* filename, long long filesize, int sock_fd)
{
    FILE *fp;
    fp = fopen(filename, "r");
    char* data = 0;
    data = (char*)malloc ((filesize + 1) * sizeof(char));
    if(data)
    {
        fread (data, sizeof(char), filesize, fp);
    }
    fclose(fp);
    
    printf("Sending File Content of %s.\n", filename);
    int n = write(sock_fd, data, filesize);
    free(data);
}

/*
    Function to print an error
*/
void error(char *err)
{
    printf("%s\n", err);
    exit(1);
}

/*
    Function to exit when write to socket fails
*/
void write_error_check(int n)
{
    if(n < 0)
    {
        error("ERROR: FAILED to write to the socket.");
    }
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

/*
    Function to check if the client executes exit
*/
int is_exit(char *buffer)
{
    regex_t regex_exit;
    if(regex_match(buffer, REG_EXIT))
    {
        return 1;
    }
    return 0;
}

/*
    Adds new line at the end of the file.
*/
void preprocess(char *filename)
{
    FILE *fp;
    char new_line = '\n';
    fp = fopen(filename, "a");
    if (fp == NULL)
    {
        error("ERROR: FAILED to open master file.");
    }
    fseek(fp, 0, SEEK_END);
    char c = getc(fp);
    if (c != '\n')
    {
        fprintf(fp, "%c", new_line);
    }
    fclose(fp);
}

/*
    Funtion to match a string to its regex pattern.
*/
int regex_match(char *string, char *regex_string)
{
    regex_t regex;
    int regex_result;
    
    /* Compile Regular Expression */
    regex_result = regcomp(&regex, regex_string, REG_EXTENDED);
    
    if(!regex_result)
    {
        /* Execute Regular Expression */
        regex_result = regexec(&regex, string, 0, NULL, 0);
        if(!regex_result)
        {
            return 1;
        }
    }
    else
    {
        printf("Regex compilation failed.\n");
    }
    return 0;
}

/*
    Function to validate the user command
*/
char* validate_command(char *buffer)
{
    regex_t regex_exit;
    int regex_result;
    char *result;
    char regex_error[100];
    
    
    if (regex_match(buffer, REG_EXIT))
    {
        result = "exit";
    }
    else if(regex_match(buffer, REG_SORT))
    {
        result = "sort";
    }
    else if(regex_match(buffer, REG_MERGE))
    {
        result = "merge";
    }
    else if(regex_match(buffer, REG_SIMILARITY))
    {
        result = "similarity";
    }
    else
    {
        result = "none";
    }
    return result;
}

long long stat_filesize(char *filename)
{
    struct stat statbuf;

    if (stat(filename, &statbuf) == -1)
    {
        printf("failed to stat %s\n", filename);
        exit(EXIT_FAILURE);
    }

    return statbuf.st_size;
}

long convert_string_to_long(char *size)
{
    char *ptr;
    long filesize = strtol(size, &ptr, 10);
    return filesize;
}
