//
//  client.c
//  W3_OnlineFileEditor
//
//  Created by Ankit Verma on 31/01/22.
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
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

const int BUFFER_SIZE = 1024;
const int READ_SIZE = 10000;

char *EXIT = "exit";
char *PRINT = "print";
char *NONE = "none";
char *APPROVED = "approved";
char *DENIED = "denied";
char *USERS = "users";
char *ALL_USERS = "allusers";
char *FILES = "files";
char *UPLOAD = "upload";
char *DOWNLOAD = "download";
char *INVITE = "invite";
char *INVITE_ANSWER_Y = "Y\n";
char *INVITE_ANSWER_N = "N\n";
char *READ = "read";
char *INSERT = "insert";
char *DELETE = "delete";
char* REG_PRINT = "^/print [a-zA-Z0-9_].*.txt\n$";
char* REG_EXIT = "^/exit\n$";
char* REG_USERS = "^/users\n$";
char* REG_ALLUSERS = "^/allusers\n$";
char* REG_FILES = "^/files\n$";
char* REG_UPLOAD = "^/upload [a-zA-Z0-9_].*.txt\n$";
char* REG_DOWNLOAD = "^/download [a-zA-Z0-9_].*.txt\n$";
char* REG_INVITE = "^/invite [a-zA-Z0-9_].*.txt [0-9]{5} [VE]{1}\n$";
char* REG_READ = "^/read [a-zA-Z0-9_].*.txt[ ]{0,1}[0-9\\-]*[ ]{0,1}[0-9\\-]*\n$";
char* REG_INSERT = "^/insert [a-zA-Z0-9_].*.txt[ ]{0,1}[0-9\\-]*[ ]{0,1}\"[A-Za-z0-9 _\\-].*\"\n$";
char* REG_DELETE = "^/delete [a-zA-Z0-9_].*.txt[ ]{0,1}[0-9\\-]*[ ]{0,1}[0-9\\-]*\n$";
char* BASE_LOCATION = "client_files/";
char* DOWNLOAD_LOCATION = "client_files/downloaded/";

void error(char *err);
void read_write_error_check(int n, int sock_fd);
long convert_string_to_long(char *size);
void signal_handler(int sig);
int regex_match(char *string, char *regex_string);
char* validate_command(char *buffer);
long long stat_filesize(char *filename);
char* create_file(char *data, char *filename);
void upload_file(int sock_fd, char* filename, long long filesize);
void download_file(char* filename, int sock_fd);
void handle_invitations(int sock_fd, char* message_from_server);

/*
    Main function
*/
int main(int argc, const char * argv[])
{
    int *agree_invitation = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    
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
    char message_to_server[BUFFER_SIZE];
    char message_from_server[10000];
    int *start = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    
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
    
    //Receive and print the unique id from the the server
    bzero(message_from_server, BUFFER_SIZE);
    n = read(sock_fd, message_from_server, BUFFER_SIZE);
    read_write_error_check(n, sock_fd);
    printf("Successfully connected to the Server.\nAssigned ClientId: %s\n", message_from_server);
    
    pid_t child_pid, wpid;
    int status = 0;
    
    start:
    if((child_pid = fork()) == 0)
    {
        char* command = validate_command(message_to_server);
        if(strcmp(command, NONE) != 0)
        {
            char bkp_command[BUFFER_SIZE];
            strcpy(bkp_command, message_to_server);
            if(strcmp(command, EXIT) == 0)
            {
                //Send /exit command to server
                n = write(sock_fd, message_to_server, strlen(message_to_server));
                read_write_error_check(n, sock_fd);
                close(sock_fd);
                error("Connection closed.");
            }
            if(strcmp(command, USERS) == 0)
            {
                //Send /users command to server
                n = write(sock_fd, message_to_server, strlen(message_to_server));
                read_write_error_check(n, sock_fd);
                
                bzero(message_from_server, BUFFER_SIZE);
                n = read(sock_fd, message_from_server, BUFFER_SIZE);
                read_write_error_check(n, sock_fd);
                
                printf("Active Clients: \n%s", message_from_server);
                
                handle_invitations(sock_fd, message_from_server);
            }
            else if(strcmp(command, PRINT) == 0)
            {
                //Send /print command to server
                n = write(sock_fd, message_to_server, strlen(message_to_server));
                read_write_error_check(n, sock_fd);
            }
            else if(strcmp(command, ALL_USERS) == 0)
            {
                //Send /allusers command to server
                n = write(sock_fd, message_to_server, strlen(message_to_server));
                read_write_error_check(n, sock_fd);
                
                bzero(message_from_server, BUFFER_SIZE);
                n = read(sock_fd, message_from_server, BUFFER_SIZE);
                read_write_error_check(n, sock_fd);
                
                printf("All Clients: \n%s", message_from_server);
            }
            else if(strcmp(command, FILES) == 0)
            {
                //Send /files command to server
                n = write(sock_fd, message_to_server, strlen(message_to_server));
                read_write_error_check(n, sock_fd);
                
                //Receive the result and print
                bzero(message_from_server, BUFFER_SIZE);
                n = read(sock_fd, message_from_server, BUFFER_SIZE);
                read_write_error_check(n, sock_fd);
                
                printf("%s\n", message_from_server);
            }
            else if(strcmp(command, UPLOAD) == 0)
            {
                //Send the /upload command to server
                n = write(sock_fd, message_to_server, strlen(message_to_server));
                read_write_error_check(n, sock_fd);
                
                //upload only if permitted, server notifies based upon the filename
                bzero(message_from_server, BUFFER_SIZE);
                n = read(sock_fd, message_from_server, BUFFER_SIZE);
                read_write_error_check(n, sock_fd);
                
                if(strncmp(APPROVED, message_from_server, 8) == 0)
                {
                    //Extract the filename from the command
                    char file[100];
                    char *filename;
                    strtok(bkp_command, " ");
                    filename = strtok(NULL, " ");
                    filename[strlen(filename) - 1] = '\0';
                    
                    strcpy(file, BASE_LOCATION);
                    strcat(file, filename);
                    
                    //STEP : Calculate filesize
                    long long filesize = stat_filesize(file);
                    if(filesize == -1)
                    {
                        printf("Failed to upload the file.\n");
                    }
                    
                    //Upload file
                    upload_file(sock_fd, file, filesize);
                }
                else
                {
                    printf("File already exists on the server. Replace operation is turned off.\n");
                }
            }
            else if(strcmp(command, DOWNLOAD) == 0)
            {
                //Send the /download command to server
                n = write(sock_fd, message_to_server, strlen(message_to_server));
                read_write_error_check(n, sock_fd);
                printf("\n"); /*DO NOT Remove this line: SEG*/
                
                //upload only if permitted, server notifies based upon the filename
                bzero(message_from_server, BUFFER_SIZE);
                n = read(sock_fd, message_from_server, BUFFER_SIZE);
                read_write_error_check(n, sock_fd);
                
                if(strncmp(APPROVED, message_from_server, 8) == 0)
                {
                    char *filename;
                    strtok(bkp_command, " ");
                    filename = strtok(NULL, " ");
                    filename[strlen(filename) - 1] = '\0';
                    download_file(filename, sock_fd);
                }
                else
                {
                    printf("The file doesn't exist or you do not have the permission to download the file.\n");
                }
            }
            else if(strcmp(command, INVITE) == 0 || strcmp(command, INVITE_ANSWER_Y) == 0 || strcmp(command, INVITE_ANSWER_N) == 0)
            {
                if(strcmp(command, INVITE_ANSWER_Y) == 0 || strcmp(command, INVITE_ANSWER_N) == 0)
                {
                    //send invitations answer to server
                    printf("Invite Answer Selected: %s\n", command);
                    n = write(sock_fd, message_to_server, strlen(message_to_server));
                    read_write_error_check(n, sock_fd);
                    printf("Invite Answer sent to server.\n");
                }
                else
                {
                    //Send /files command to server
                    n = write(sock_fd, message_to_server, strlen(message_to_server));
                    read_write_error_check(n, sock_fd);
                    printf("Invitation is sent to the client.\nClient will be able to view this invitation after his command is executed.\n");
                }
            }
            else if(strcmp(command, READ) == 0)
            {
                //Send /insert command to server
                n = write(sock_fd, message_to_server, strlen(message_to_server));
                read_write_error_check(n, sock_fd);
                
                //Reading the contents from the server
                bzero(message_from_server, READ_SIZE);
                n = read(sock_fd, message_from_server, READ_SIZE);
                read_write_error_check(n, sock_fd);
                printf("%s\n", message_from_server);
                 
            }
            else if(strcmp(command, INSERT) == 0)
            {
                //Send /insert command to server
                n = write(sock_fd, message_to_server, strlen(message_to_server));
                read_write_error_check(n, sock_fd);
                
                bzero(message_from_server, BUFFER_SIZE);
                n = read(sock_fd, message_from_server, BUFFER_SIZE);
                read_write_error_check(n, sock_fd);
                printf("%s\n", message_from_server);
            }
            else if(strcmp(command, DELETE) == 0)
            {
                //Send /insert command to server
                n = write(sock_fd, message_to_server, strlen(message_to_server));
                read_write_error_check(n, sock_fd);
             
                bzero(message_from_server, BUFFER_SIZE);
                n = read(sock_fd, message_from_server, BUFFER_SIZE);
                read_write_error_check(n, sock_fd);
                printf("%s\n", message_from_server);
            }
        }
        else
        {
            if(*start != 0)
            {
                printf("Invalid Command.\n");
            }
            
        }
        exit(0);
    }
    else
    {
        int returnStatus = -1000;
        bzero(message_to_server, BUFFER_SIZE);
        sleep(1);
        
        printf("\nInput:");
        fgets(message_to_server, BUFFER_SIZE, stdin);
        *start = *start + 1;
        
        //If child is exits, start again
        waitpid(child_pid, &returnStatus, 0);
        if(returnStatus == 0)
        {
            goto start;
        }
    }
    return 0;
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
void read_write_error_check(int n, int sock_fd)
{
    if(n < 0)
    {
        close(sock_fd);
        error("ERROR: FAILED to read/write to the socket.");
    }
}

long convert_string_to_long(char *size)
{
    char *ptr;
    long filesize = strtol(size, &ptr, 10);
    return filesize;
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
    
    if(strncmp(buffer,"Y",1) == 0 || strncmp(buffer,"N",1) == 0)
    {
        return buffer;
    }
    else if (regex_match(buffer, REG_EXIT))
    {
        result = "exit";
    }
    else if(regex_match(buffer, REG_USERS))
    {
        result = "users";
    }
    else if(regex_match(buffer, REG_ALLUSERS))
    {
        result = "allusers";
    }
    else if(regex_match(buffer, REG_FILES))
    {
        result = "files";
    }
    else if(regex_match(buffer, REG_UPLOAD))
    {
        result = "upload";
    }
    else if(regex_match(buffer, REG_DOWNLOAD))
    {
        result = "download";
    }
    else if(regex_match(buffer, REG_INVITE))
    {
        result = "invite";
    }
    else if(regex_match(buffer, REG_READ))
    {
        result = "read";
    }
    else if(regex_match(buffer, REG_INSERT))
    {
        result = "insert";
    }
    else if(regex_match(buffer, REG_DELETE))
    {
        result = "delete";
    }
    else if(regex_match(buffer, REG_PRINT))
    {
        result = "print";
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
        printf("Failed to calculate the filesize of %s\n", filename);
        return -1;
    }

    return statbuf.st_size;
}

char* create_file(char *data, char *filename)
{
    char *file_location;
    file_location = (char*)malloc(BUFFER_SIZE);
    strcpy(file_location, DOWNLOAD_LOCATION);
    strcat(file_location, filename);
    //printf("Filelocation: %s\n", file_location);
    FILE *fp = fopen(file_location, "w");
    if (fp == NULL)
    {
        error("ERROR: FAILED to create file.\n");
    }
    int res = fputs(data, fp);
    fclose(fp);
    
    return (char*)file_location;
}

void upload_file(int sock_fd, char* filename, long long filesize)
{
    int n;
    
    //Send the filesize
    char fsize[BUFFER_SIZE];
    sprintf(fsize, "%lld", filesize);
    n = write(sock_fd, fsize, strlen(fsize));
    read_write_error_check(n, sock_fd);
    
    //Send the data
    FILE *fp;
    fp = fopen(filename, "r");
    char* data = 0;
    data = (char*)malloc ((filesize + 1) * sizeof(char));
    if(data)
    {
        fread (data, sizeof(char), filesize, fp);
    }
    fclose(fp);
    
    printf("Uploading File Content of %s.\n", filename);
    n = write(sock_fd, data, filesize);
    free(data);
}

void download_file(char* filename, int sock_fd)
{
    printf("Downloading file...\n");
    int n;
    char* data;
    char message_from_client[BUFFER_SIZE];
    bzero(message_from_client, BUFFER_SIZE);
    n = read(sock_fd, message_from_client, BUFFER_SIZE);
    read_write_error_check(n, sock_fd);
    
    //Convert the string filesize to long
    long filesize = convert_string_to_long(message_from_client);
    
    //Read the content from the pipe
    bzero(data, filesize);
    data = (char*)malloc ((filesize+1)*sizeof(char));
    n = read(sock_fd, data, filesize);
    read_write_error_check(n, sock_fd);
    
    //Create a file for the content
    printf("File downloaded at: %s\n", create_file(data ,filename));
     
}

void handle_invitations(int sock_fd, char* message_from_server)
{
    int n;
    char message_to_server[BUFFER_SIZE];
    bzero(message_from_server, BUFFER_SIZE);
    n = read(sock_fd, message_from_server, BUFFER_SIZE);
    read_write_error_check(n, sock_fd);
    
    if(strncmp(message_from_server, "invited", 7) == 0)
    {
        printf("\nYou have pending invitations.\n");
        
        bzero(message_from_server, BUFFER_SIZE);
        n = read(sock_fd, message_from_server, BUFFER_SIZE);
        read_write_error_check(n, sock_fd);
        printf("%s\n", message_from_server);
        printf("Do you want to accept above invitation(Y/N)?\n");
        
        //char option[BUFFER_SIZE];
        //fgets(option, sizeof(option), stdin);
        /*
        if (fgets(option, sizeof(option), stdin) == 0)
        {
            fprintf(stderr, "Failed to read your options\n");
            exit(0);
        }
        option[strcspn(option, "\n")] = '\0';
        printf("\n");
        */
        
        //TODO: Implement user input
        //bzero(message_to_server, BUFFER_SIZE);
        //strcpy(message_to_server, "Y"); //fgets not working :(
        //n = write(sock_fd, message_to_server, strlen(message_to_server));
        //read_write_error_check(n, sock_fd);
        
    }
    else
    {
        printf("\nYou do not have pending invitations.\n");
    }
}
