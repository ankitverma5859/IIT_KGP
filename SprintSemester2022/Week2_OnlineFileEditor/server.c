//
//  server.c
//  W3_OnlineFileEditor
//
//  Created by Ankit Verma on 31/01/22.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <regex.h>
#include <sys/stat.h>
#include <time.h>

const int BUFFER_SIZE = 256;
const int MAX_CLIENTS = 5;

char *EXIT = "exit";
char *NONE = "none";
char *USERS = "users";
char *ALL_USERS = "allusers";
char *FILES = "files";
char *UPLOAD = "upload";
char *DOWNLOAD = "download";
char *INVITE = "invite";
char *READ = "read";
char *INSERT = "insert";
char *DELETE = "delete";
char* REG_EXIT = "^/exit\n$";
char* REG_USERS = "^/users\n$";
char* REG_ALLUSERS = "^/allusers\n$";
char* REG_FILES = "^/files\n$";
char* REG_UPLOAD = "^/upload [a-zA-Z0-9_].*.txt\n$";
char* REG_DOWNLOAD = "^/download [a-zA-Z0-9_].*.txt\n$";
char* REG_INVITE = "^/invite [a-zA-Z0-9_].*.txt [0-9]{5} [V,E]{1}\n$";
char* REG_READ = "^/read [a-zA-Z0-9_].*.txt[ ]{0,1}[0-9\\-]*[ ]{0,1}[0-9\\-]*\n$";
char* REG_INSERT = "^/insert [a-zA-Z0-9_].*.txt[ ]{0,1}[0-9\\-]*[ ]{0,1}[A-Za-z0-9 _\\-].*\n$";
char* REG_DELETE = "^/delete [a-zA-Z0-9_].*.txt[ ]{0,1}[0-9\\-]*[ ]{0,1}[0-9\\-]*\n$";
char* CLIENTS_FILE = "server_files/clients.txt";
char* CLIENTS_DATA = "server_files/client_uploads/";
char* INVITATION_FILE = "server_files/invitations.txt";
char* FILES_METADATA = "server_files/files_metadata.txt";

struct CLIENT
{
    int client_id;
    int active_status;
    char hostname[100];
    int port;
    int sock_fd;
};

struct FILE_METADATA
{
    char filename[50];
    int filesize;
    int owner;
};

struct INVITATION
{
    int sockfd;
    int from_port;
    int to_port;
    char filename[100];
    char permission[5];
    int is_active;
};

void error(char *msg);
void signal_handler(int sig);
void read_write_error_check(int n, int sock_fd, int *client_count);
long convert_string_to_long(char *size);
int regex_match(char *string, char *regex_string);
char* validate_command(char *buffer);
int generate_client_id();
int calculate_num_of_lines(char* filelocation);
long long stat_filesize(char *filename);
char* create_file(char *data, char *filename);
int is_clientid_exists(int client_id);
struct CLIENT *parse_client_file();
void register_client(char* client_id, char* hostname, int port, int sock_fd);
void update_active_users(int client_id);
char* active_users();
char* all_users();
void download_file(char* filename, int sock_fd, int *client_count);
void upload_file(int sock_fd, char* filename, long long filesize, int *client_count);
void update_file_metadata(char* filename, int client_id);
int is_owner(char* filename, int client_number);
struct CLIENT get_client_detail(char* client_id);
int convert_string_to_int(char *string);
void add_invitation(int sock_fd, int from_port, int to_port, char* filename, char* permission);
int has_invitation(int sockfd);
void check_invitations(int sock_fd, int *client_count);
/*
    Main Function
*/
int main(int argc, char * argv[])
{
    // Initialize Clients file to empty when server starts
    FILE *fp = fopen(CLIENTS_FILE, "w");
    fclose(fp);
    FILE *fp1 = fopen(FILES_METADATA, "w");
    fclose(fp1);
    FILE *fp2 = fopen(INVITATION_FILE, "w");
    fclose(fp2);
    
    printf("Give Start message to user:\n");
    signal(SIGINT, signal_handler);
    
    int *client_count = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    int sock_fd, newsock_fd, port_no;
    pid_t child_pid, child_pid1;
    
    socklen_t client_len;
    struct sockaddr_in server_addr, client_addr;
    
    char buffer[BUFFER_SIZE];
    char message_to_client[BUFFER_SIZE], message_from_client[BUFFER_SIZE];
    
    if (argc < 2)
    {
        error("ERROR: PORT not provided!\n");
    }
    
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sock_fd < 0)
    {
        error("ERROR: FAILED to open socket.");
    }
    
    bzero((char *) &server_addr, sizeof(server_addr));
    port_no = atoi(argv[1]);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_no);
    
    if(bind(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        error("ERROR: FAILED to bind.");
    }
    
    if(listen(sock_fd, 5) == 0)
    {
        printf("Server is running.\n");
    }
    else
    {
        error("ERROR: FAILED to execute listen.\n");
    }
    
    while(1)
    {
        client_len = sizeof(client_addr);
        
        newsock_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &client_len);
        
        if (newsock_fd < 0)
        {
            exit(1);
        }
        
        if (*client_count < MAX_CLIENTS)
        {
            *client_count = *client_count + 1;
            
            if((child_pid = fork()) == 0)
            {
                int n;
                
                printf("\nConnection Accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                
                //Generate the 5 digit code for the client
                int client_number = generate_client_id();
                char client_id[6];
                sprintf(client_id, "%d", client_number);
                printf("ClientId Generated: %s\n", client_id);
                
                register_client(client_id, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), newsock_fd);
                
                //Sent the code to the client
                bzero(message_to_client, BUFFER_SIZE);
                strcpy(message_to_client, client_id);
                n = write(newsock_fd, message_to_client, strlen(message_to_client) + 1);
                read_write_error_check(n, newsock_fd, client_count);
                
                close(sock_fd);
                while(1)
                {
                    //Clients request is processed here.
                    bzero(message_from_client, BUFFER_SIZE);
                    n = read(newsock_fd, message_from_client, BUFFER_SIZE);
                    read_write_error_check(n, newsock_fd, client_count);
                    
                    char master_command[BUFFER_SIZE];
                    strcpy(master_command, message_from_client);
                    char *command_to_execute = validate_command(message_from_client);
                    if(strcmp(command_to_execute, EXIT) == 0)
                    {
                        update_active_users(client_number);
                        printf("Disconnected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                        *client_count = *client_count - 1;
                        printf("Number of Concurrent Clients: %d\n", *client_count);
                        break;
                    }
                    else
                    {
                        if(strcmp(command_to_execute, NONE) == 0)
                        {
                            update_active_users(client_number);
                            printf("Client abruptly closed.\n");
                            printf("Disconnected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                            *client_count = *client_count - 1;
                            printf("Number of Concurrent Clients: %d\n", *client_count);
                            break;
                        }
                        else if(strcmp(command_to_execute, USERS) == 0)
                        {
                            printf("Processing %s\n", master_command);
                            bzero(message_to_client, BUFFER_SIZE);
                            strcpy(message_to_client, active_users());
                            n = write(newsock_fd, message_to_client, strlen(message_to_client) + 1);
                            read_write_error_check(n, newsock_fd, client_count);
                            
                            check_invitations(newsock_fd, client_count);
                            /*
                            if(has_invitation(newsock_fd))
                            {
                                printf("Checking for invitation.\n");
                                //message_to_client = "invited";
                                bzero(message_to_client, BUFFER_SIZE);
                                strcpy(message_to_client, "invited");
                                n = write(newsock_fd, message_to_client, strlen(message_to_client) + 1);
                                read_write_error_check(n, newsock_fd, client_count);
                            }
                            else
                            {
                                printf("invitation not found.\n");
                                //message_to_client = "no";
                                bzero(message_to_client, BUFFER_SIZE);
                                strcpy(message_to_client, "no");
                                n = write(newsock_fd, message_to_client, strlen(message_to_client) + 1);
                                read_write_error_check(n, newsock_fd, client_count);
                            }
                             */
                        }
                        else if(strcmp(command_to_execute, ALL_USERS) == 0)
                        {
                            printf("Processing %s\n", master_command);
                            bzero(message_to_client, BUFFER_SIZE);
                            strcpy(message_to_client, all_users());
                            n = write(newsock_fd, message_to_client, strlen(message_to_client) + 1);
                            read_write_error_check(n, newsock_fd, client_count);
                        }
                        else if(strcmp(command_to_execute, UPLOAD) == 0)
                        {
                            printf("Processing %s\n", master_command);
                            
                            char *filename;
                            strtok(master_command, " ");
                            filename = strtok(NULL, " ");
                            filename[strlen(filename) - 1] = '\0';
                            
                            if(is_owner(filename, client_number))  //Also need to check collaborator permission is_collaborator()
                            {
                                bzero(message_to_client, BUFFER_SIZE);
                                strcpy(message_to_client, "approved");
                                n = write(newsock_fd, message_to_client, strlen(message_to_client) + 1);
                                read_write_error_check(n, newsock_fd, client_count);
                                
                                download_file(filename, newsock_fd, client_count);
                                update_file_metadata(filename, client_number);
                            }
                            else
                            {
                                //Send not allowed to upload. File of same name exits
                                bzero(message_to_client, BUFFER_SIZE);
                                strcpy(message_to_client, "denied");
                                n = write(newsock_fd, message_to_client, strlen(message_to_client) + 1);
                                read_write_error_check(n, newsock_fd, client_count);
                            }
                        }
                        else if(strcmp(command_to_execute, DOWNLOAD) == 0) //Also need to check collaborator permission is_collaborator()
                        {
                            printf("Processing %s\n", master_command);
                            
                            //Extract the filename from the command
                            char file[100];
                            char *filename;
                            strtok(master_command, " ");
                            filename = strtok(NULL, " ");
                            filename[strlen(filename) - 1] = '\0';
                            
                            if(is_owner(filename, client_number))
                            {
                                strcpy(file, CLIENTS_DATA);
                                strcat(file, filename);
                                
                                bzero(message_to_client, BUFFER_SIZE);
                                strcpy(message_to_client, "approved");
                                n = write(newsock_fd, message_to_client, strlen(message_to_client) + 1);
                                read_write_error_check(n, newsock_fd, client_count);
                                
                                //STEP : Calculate filesize
                                long long filesize = stat_filesize(file);
                                if(filesize == -1)
                                {
                                    printf("Failed to upload the file.\n");
                                }
                                //People with editor permission should be able to download the file
                                upload_file(newsock_fd, file, filesize, client_count);
                            }
                            else
                            {
                                //Send not allowed to upload. File of same name exits
                                bzero(message_to_client, BUFFER_SIZE);
                                strcpy(message_to_client, "denied");
                                n = write(newsock_fd, message_to_client, strlen(message_to_client) + 1);
                                read_write_error_check(n, newsock_fd, client_count);
                            }
                            
                            
                        }
                        else if(strcmp(command_to_execute, FILES) == 0)
                        {
                            printf("Processing %s\n", master_command);
                            
                            //Process
                            //get_files();
                        }
                        else if(strcmp(command_to_execute, INVITE) == 0)
                        {
                            printf("Processing %s\n", master_command);
                            
                            char *command;
                            char *filename;
                            char *client_id;
                            char *permission;
                            strtok(master_command, " ");
                            filename = strtok(NULL, " ");
                            client_id = strtok(NULL, " ");
                            permission = strtok(NULL, " ");
                            permission[strlen(permission) - 1] = '\0';
                            printf("Filename: %s ClientId: %s Permission: %s\n", filename, client_id, permission);
                            
                            
                            struct CLIENT client = get_client_detail(client_id);
                            add_invitation(client.sock_fd, ntohs(client_addr.sin_port), client.port, filename, permission);
                            printf("Invitation queued for clients approval.\n");
                            //bzero(message_to_client, BUFFER_SIZE);
                            //n = write(newsock_fd, message_to_client, strlen(message_to_client) + 1);
                            //read_write_error_check(n, newsock_fd, client_count);
                        }
                        
                        bzero(message_from_client, BUFFER_SIZE);
                        bzero(message_to_client, BUFFER_SIZE);
                    }
                    
                }
            }
        }
        else
        {
            printf("Server has reached maximum client limit.\n");
            char *msg = "max";
            int n = write(newsock_fd, msg, strlen(msg) + 1);
            close(newsock_fd);
        }
    }
           
    close(newsock_fd);
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
    Function to exit when write to socket fails
*/
void read_write_error_check(int n, int sock_fd, int *client_count)
{
    if(n < 0)
    {
        *client_count = *client_count - 1;
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
    
    if (regex_match(buffer, REG_EXIT))
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
    else
    {
        result = "none";
    }
    return result;
}

int generate_client_id()
{
    int upper = 99999;
    int lower = 10000;
    srand(time(0));
    
    int client_id = (rand() % (upper - lower)) + lower;
    while(is_clientid_exists(client_id))
    {
        client_id = (rand() % (upper - lower)) + lower;
    }
          return client_id;
    //check if the id already exists in the client list or else generate again
}

int calculate_num_of_lines(char* filelocation)
{
    char buffer[BUFFER_SIZE];
    FILE *fp = fopen(filelocation, "r");
    int lines = 0;
    while(fscanf(fp, "%[^\n]\n", buffer) != EOF)
    {
        lines = lines + 1;
    }
    fclose(fp);
    return lines;
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
    strcpy(file_location, CLIENTS_DATA);
    strcat(file_location, filename);
    FILE *fp = fopen(file_location, "w");
    if (fp == NULL)
    {
        error("ERROR: FAILED to create file.\n");
    }
    int res = fputs(data, fp);
    fclose(fp);
    
    return (char*)file_location;
}

int is_clientid_exists(int client_id)
{
    int num_of_lines = calculate_num_of_lines(CLIENTS_FILE);
    struct CLIENT* client_list = parse_client_file(num_of_lines);
    for(int i = 0; i < num_of_lines; i++)
    {
        if(client_list[i].client_id == client_id)
        {
            return 1;
        }
    }
    return 0;
}

struct CLIENT *parse_client_file(int num_of_lines)
{
    char client_buffer[BUFFER_SIZE];
    struct CLIENT *client_list = malloc(sizeof(struct CLIENT) * num_of_lines);
    
    FILE *fp = fopen(CLIENTS_FILE, "r");
    if (fp == NULL)
    {
        error("ERROR: FAILED to open clients file.");
    }
    for(int i = 0; i < num_of_lines && fscanf(fp, "%[^\n]\n", client_buffer) != EOF; i++)
    {
        int n = sscanf(client_buffer, "%d\t%s\t%d\t%d\t%d\n",
                       &client_list[i].client_id,
                       client_list[i].hostname,
                       &client_list[i].port,
                       &client_list[i].active_status,
                       &client_list[i].sock_fd);
    }
    fclose(fp);
    return client_list;
}

struct FILE_METADATA *parse_filemeta_file(int num_of_lines)
{
    char filemeta_buffer[BUFFER_SIZE];
    struct FILE_METADATA *filemeta_list = malloc(sizeof(struct FILE_METADATA) * num_of_lines);
    
    FILE *fp = fopen(FILES_METADATA, "r");
    if (fp == NULL)
    {
        error("ERROR: FAILED to open clients file.");
    }
    for(int i = 0; i < num_of_lines; i++)
    {
        fscanf(fp, "%[^\n]\n", filemeta_buffer);
        int n = sscanf(filemeta_buffer, "%s\t%d\t%d\n",
                       filemeta_list[i].filename,
                       &filemeta_list[i].owner,
                       &filemeta_list[i].filesize);
        bzero(filemeta_buffer, BUFFER_SIZE);
    }
    fclose(fp);
    return filemeta_list;
}

struct INVITATION *parse_invitation_file(int num_of_lines)
{
    char invitation_buffer[BUFFER_SIZE];
    struct INVITATION *invitation_list = malloc(sizeof(struct INVITATION) * num_of_lines);
    
    FILE *fp = fopen(INVITATION_FILE, "r");
    if (fp == NULL)
    {
        error("ERROR: FAILED to open clients file.");
    }
    for(int i = 0; i < num_of_lines; i++)
    {
        fscanf(fp, "%[^\n]\n", invitation_buffer);
        int n = sscanf(invitation_buffer, "%d\t%d\t%d\t%s\t%s\t%d\n",
                       &invitation_list[i].sockfd,
                       &invitation_list[i].from_port,
                       &invitation_list[i].to_port,
                       invitation_list[i].filename,
                       invitation_list[i].permission,
                       &invitation_list[i].is_active
                       );
        bzero(invitation_buffer, BUFFER_SIZE);
    }
    fclose(fp);
    return invitation_list;
}

void register_client(char* client_id, char* hostname, int port, int sock_fd)
{
    FILE *fp;
    fp = fopen(CLIENTS_FILE, "a+");
    if (fp == NULL)
    {
        error("ERROR: FAILED to open clients file.");
    }
    fprintf(fp, "%s\t%s\t%d\t1\t%d\n", client_id, hostname, port, sock_fd);
    fclose(fp);
}

int has_invitation(int sockfd)
{
    int num_of_lines = calculate_num_of_lines(INVITATION_FILE);
    struct INVITATION* invitation_list = parse_invitation_file(num_of_lines);
    
    for(int i = 0; i < num_of_lines; i++)
    {
        if(sockfd == invitation_list[i].sockfd && invitation_list[i].is_active == 1)
        {
            return 1;
        }
    }
    
    return 0;
}

void update_active_users(int client_id)
{
    int num_of_lines = calculate_num_of_lines(CLIENTS_FILE);
    struct CLIENT* client_list = parse_client_file(num_of_lines);
    
    FILE *fp1 = fopen(CLIENTS_FILE, "w");
    for(int i = 0; i < num_of_lines; i++)
    {
       if(client_list[i].client_id == client_id)
       {
           fprintf(fp1, "%d\t%s\t%d\t0\t%d\n", client_list[i].client_id, client_list[i].hostname, client_list[i].port, client_list[i].sock_fd);
       }
       else
       {
           fprintf(fp1, "%d\t%s\t%d\t%d\t%d\n", client_list[i].client_id, client_list[i].hostname, client_list[i].port, client_list[i].active_status, client_list[i].sock_fd);
       }
    }
    fclose(fp1);
}

char* active_users()
{
    char* a_clients;
    strcpy(a_clients, "");
    int num_of_lines = calculate_num_of_lines(CLIENTS_FILE);
    struct CLIENT* client_list = parse_client_file(num_of_lines);
    for(int i = 0; i < num_of_lines; i++)
    {
        if(client_list[i].active_status == 1)
        {
            char client_id[6];
            sprintf(client_id, "%d", client_list[i].client_id);
            strcat(a_clients, client_id);
            strcat(a_clients, "\n");
        }
    }
    return a_clients;
}

char* all_users()
{
    char* a_clients;
    strcpy(a_clients, "");
    int num_of_lines = calculate_num_of_lines(CLIENTS_FILE);
    struct CLIENT* client_list = parse_client_file(num_of_lines);
    for(int i = 0; i < num_of_lines; i++)
    {
        char client_id[6];
        sprintf(client_id, "%d", client_list[i].client_id);
        strcat(a_clients, client_id);
        strcat(a_clients, "\t");
        
        if(client_list[i].active_status == 1)
        {
            strcat(a_clients, "active");
        }
        else
        {
            strcat(a_clients, "in-active");
        }
        strcat(a_clients, "\n");
    }
    return a_clients;
}

void download_file(char* filename, int sock_fd, int *client_count)
{
    int n;
    char message_from_client[BUFFER_SIZE];
    bzero(message_from_client, BUFFER_SIZE);
    n = read(sock_fd, message_from_client, BUFFER_SIZE);
    read_write_error_check(n, sock_fd, client_count);
    
    //Convert the string filesize to long
    long filesize = convert_string_to_long(message_from_client);
    
    //Read the content from the pipe
    char* data = (char*)malloc ((filesize+1)*sizeof(char));
    n = read(sock_fd, data, filesize);
    read_write_error_check(n, sock_fd, client_count);
    
    //Create a file for the content
    printf("File stored at: %s\n", create_file(data, filename));
}

void upload_file(int sock_fd, char* filename, long long filesize, int *client_count)
{
    int n;
    
    //Send the filesize
    char fsize[BUFFER_SIZE];
    sprintf(fsize, "%lld", filesize);
    n = write(sock_fd, fsize, strlen(fsize));
    read_write_error_check(n, sock_fd, client_count);
    
    
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
    
    printf("Sending File Content of %s.\n", filename);
    n = write(sock_fd, data, filesize);
    read_write_error_check(n, sock_fd, client_count);
    free(data);
    
}

void update_file_metadata(char* filename, int client_id)
{
    char *file_location;
    file_location = (char*)malloc(BUFFER_SIZE);
    strcpy(file_location, CLIENTS_DATA);
    strcat(file_location, filename);
    
    FILE* fp = fopen(FILES_METADATA, "a+");
    if (fp == NULL)
    {
        error("ERROR: FAILED to open files_metadata file.");
    }
    long long filesize = stat_filesize(file_location);
    fprintf(fp, "%s\t%d\t%lld\n", filename, client_id, filesize);
    fclose(fp);
}

int is_owner(char* filename, int client_number)
{
    int num_of_lines = calculate_num_of_lines(FILES_METADATA);
    struct FILE_METADATA* file_meta = parse_filemeta_file(num_of_lines);
    
    for(int i = 0; i < num_of_lines; i++)
    {
        if(strcmp(file_meta[i].filename, filename) == 0 && file_meta[i].owner != client_number)
        {
            return 0;
        }
    }
    return 1;
}

struct CLIENT get_client_detail(char* client_id)
{
    struct CLIENT *client = malloc(sizeof(struct CLIENT));
    
    int num_of_lines = calculate_num_of_lines(CLIENTS_FILE);
    struct CLIENT* client_list = parse_client_file(num_of_lines);
    
    for(int i = 0; i < num_of_lines; i++)
    {
        if(client_list[i].client_id == convert_string_to_int(client_id))
        {
            *client = client_list[i];
        }
    }
    return *client;
}

int convert_string_to_int(char *string)
{
    char *ptr;
    int integer_value = strtol(string, &ptr, 10);
    return integer_value;
}

void add_invitation(int sock_fd, int from_port, int to_port, char* filename, char* permission)
{
    FILE* fp = fopen(INVITATION_FILE, "a+");
    if (fp == NULL)
    {
        error("ERROR: FAILED to invitation list file.");
    }
    fprintf(fp, "%d\t%d\t%d\t%s\t%s\t1\n", sock_fd, from_port, to_port, filename, permission);
    fclose(fp);

}


void check_invitations(int sock_fd, int *client_count)
{
    int n;
    char message_to_client[BUFFER_SIZE];
    if(has_invitation(sock_fd))
    {
        printf("Invitations Found.\n");
        bzero(message_to_client, BUFFER_SIZE);
        strcpy(message_to_client, "invited");
        n = write(sock_fd, message_to_client, strlen(message_to_client) + 1);
        read_write_error_check(n, sock_fd, client_count);
    }
    else
    {
        printf("No Invitations Found.\n");
        bzero(message_to_client, BUFFER_SIZE);
        strcpy(message_to_client, "no");
        n = write(sock_fd, message_to_client, strlen(message_to_client) + 1);
        read_write_error_check(n, sock_fd, client_count);
    }
}
