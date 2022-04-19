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

const int MAX_INT = 32766;
const int BUFFER_SIZE = 256;
const int MAX_CLIENTS = 5;
const int READ_SIZE = 10000;

char *EXIT = "exit";
char *PRINT = "print";
char *NONE = "none";
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
char* REG_DIGIT = "^[0-9\\-].*$";
char* REG_EXIT = "^/exit\n$";
char* REG_USERS = "^/users\n$";
char* REG_ALLUSERS = "^/allusers\n$";
char* REG_FILES = "^/files\n$";
char* REG_UPLOAD = "^/upload [a-zA-Z0-9_].*.txt\n$";
char* REG_DOWNLOAD = "^/download [a-zA-Z0-9_].*.txt\n$";
char* REG_INVITE = "^/invite [a-zA-Z0-9_].*.txt [0-9]{5} [V,E]{1}\n$";
char* REG_READ = "^/read [a-zA-Z0-9_].*.txt[ ]{0,1}[0-9\\-]*[ ]{0,1}[0-9\\-]*\n$";
char* REG_INSERT = "^/insert [a-zA-Z0-9_].*.txt[ ]{0,1}[0-9\\-]*[ ]{0,1}\"[A-Za-z0-9 _\\-].*\"\n$";
char* REG_DELETE = "^/delete [a-zA-Z0-9_].*.txt[ ]{0,1}[0-9\\-]*[ ]{0,1}[0-9\\-]*\n$";
char* CLIENTS_FILE = "server_files/clients.txt";
char* CLIENTS_DATA = "server_files/client_uploads/";
char* INVITATION_FILE = "server_files/invitations.txt";
char* FILES_METADATA = "server_files/files_metadata.txt";
char* COLLABORATORS_FILE = "server_files/collaborators.txt";
char* REG_INSERT_NOINDEX = "^/insert [a-zA-Z0-9_].*.txt[ ]{0,1}\"[A-Za-z0-9 _\\-].*\"\n$";

struct CLIENT
{
    int client_id;
    int active_status;
    char hostname[100];
    int port;
    int sock_fd;
};

struct COLLABORATOR
{
    char filename[50];
    int client_id;
    char permission[5];
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
void check_invitations(int sock_fd, int *client_count, int client_id);
char* get_invitations(int sock_fd);
void turn_off_invitation(int sock_fd);
void activate_invitation(int sock_fd, int client_id);
struct COLLABORATOR *parse_collaborator_file(int num_of_lines);
int is_collaborator(char* filename, int client_number);
char* get_files();
char* insert_in_file(char* filename, char* index, char* message);
void display_file_content(char* filename);
void append_file(char* file_location, char* message);
int is_file_exists(char* filename);
char* delete(char* file_location, int s_index, int e_index);
char* read_file(char* file_location, int s_index, int e_index);

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
    FILE *fp3 = fopen(COLLABORATORS_FILE, "w");
    fclose(fp3);
    
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
                            
                            check_invitations(newsock_fd, client_count, client_number);
                            
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
                            
                            if(!is_file_exists(filename))
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
                            
                            if(is_file_exists(filename) && (is_owner(filename, client_number) || (is_collaborator(filename, client_number) == 2) || (is_collaborator(filename, client_number) == 1)))
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
                            char* result = get_files();
                            
                            bzero(message_to_client, BUFFER_SIZE);
                            strcpy(message_to_client, result);
                            n = write(newsock_fd, message_to_client, strlen(message_to_client) + 1);
                            read_write_error_check(n, newsock_fd, client_count);
                            
                        }
                        else if(strcmp(command_to_execute, PRINT) == 0)
                        {
                            printf("Processing %s\n", master_command);
                            char *filename;
                            strtok(master_command, " ");
                            filename = strtok(NULL, " ");
                            filename[strlen(filename) - 1] = '\0';
                            
                            display_file_content(filename);
                        }
                        else if(strcmp(command_to_execute, INVITE) == 0 || strcmp(command_to_execute, INVITE_ANSWER_Y) == 0 || strcmp(command_to_execute, INVITE_ANSWER_N) == 0)
                        {
                            if(strcmp(command_to_execute, INVITE_ANSWER_Y) == 0 || strcmp(command_to_execute, INVITE_ANSWER_N) == 0)
                            {
                                printf("Clients Answers received: %s\n", command_to_execute);
                                if(strcmp(command_to_execute, INVITE_ANSWER_Y) == 0)
                                {
                                    printf("Colllaboration activated.\n");
                                    activate_invitation(newsock_fd, client_number);
                                }
                                else if(strcmp(command_to_execute, INVITE_ANSWER_N) == 0)
                                {
                                    printf("Colllaboration deactivated.\n");
                                    turn_off_invitation(newsock_fd);
                                }
                            }
                            else
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
                            }
                            
                        }
                        else if(strcmp(command_to_execute, INSERT) == 0)
                        {
                            printf("Processing %s\n", master_command);
                            
                            char* result;
                            char *filename;
                            char *index;
                            char *message;
                            char msg[200];
                            strcpy(msg, "");
                            strcat(msg, master_command);
                            
                            
                            if(regex_match(msg, REG_INSERT_NOINDEX))
                            {
                                strtok(msg, " ");
                                filename = strtok(NULL, " ");
                                message = strtok(NULL, "");
                           }
                            else
                            {
                                strtok(msg, " ");
                                filename = strtok(NULL, " ");
                                index = strtok(NULL, " ");
                                message = strtok(NULL, "");
                            }
                            
                            if(is_file_exists(filename) && (is_owner(filename, client_number) || (is_collaborator(filename, client_number) == 2)))
                            {
                                if(!regex_match(master_command, REG_INSERT_NOINDEX))
                                {
                                    result = insert_in_file(filename, index, message);
                                    //result = "Insert Successful.\n";
                                }
                                else
                                {
                                    result = insert_in_file(filename, "X", message);//here, is_index is the message
                                    //result = "Insert Successful.\n";
                                }
                            }
                            else
                            {
                                result = "The file doesn't exist or you do not have the permission to edit the file.\n";
                            }
                            bzero(message_to_client, BUFFER_SIZE);
                            strcpy(message_to_client, result);
                            n = write(newsock_fd, message_to_client, strlen(message_to_client) + 1);
                            read_write_error_check(n, newsock_fd, client_count);
                        }
                        else if(strcmp(command_to_execute, DELETE) == 0)
                        {
                            printf("Processing %s\n", master_command);
                            char* result;
                            char cmd[50];
                            char filename[100];
                            int s_idx;
                            int e_idx;
                            
                            int var = sscanf(master_command, "%s %s %d %d\n", cmd, filename, &s_idx, &e_idx);
                            if(is_file_exists(filename) && (is_owner(filename, client_number) || (is_collaborator(filename, client_number) == 2)))
                            {
                                char *file_location;
                                file_location = (char*)malloc(BUFFER_SIZE);
                                strcpy(file_location, CLIENTS_DATA);
                                strcat(file_location, filename);
                                
                                int total_lines = calculate_num_of_lines(file_location);
                                if(var == 4)
                                {
                                    if(s_idx >= -(total_lines) && s_idx <= total_lines && e_idx >= -(total_lines) && e_idx <= total_lines)
                                    {
                                        if((s_idx <= 0 && e_idx <= 0) || (s_idx >= 0 && e_idx >= 0))
                                        {
                                            if (s_idx < 0 && e_idx < 0)
                                            {
                                                s_idx = total_lines + s_idx;
                                                e_idx = total_lines + e_idx;
                                            }
                                            result = delete(file_location, s_idx, e_idx);
                                        }
                                        else
                                        {
                                            result = "ERROR: Index out of Range.\n";
                                        }
                                    }
                                    else
                                    {
                                        result = "ERROR: Index out of Range.\n";
                                    }
                                }
                                else if(var == 3)
                                {
                                    if(s_idx >= -(total_lines) && s_idx <= total_lines)
                                    {
                                        if (s_idx < 0)
                                        {
                                            s_idx = total_lines + s_idx;
                                        }
                                        result = delete(file_location, s_idx, MAX_INT);
                                    }
                                    else
                                    {
                                        result = "ERROR: Index out of Range.\n";
                                    }
                                }
                                else
                                {
                                    printf("Deleting all the contents of the file.\n");
                                    FILE* fp = fopen(file_location, "w");
                                    fclose(fp);
                                    result = "All contents of the files is deleted.\n";
                                }
                            }
                            else
                            {
                                result = "The file doesn't exist or you do not have the permission to delete the file.\n";
                            }
                            bzero(message_to_client, BUFFER_SIZE);
                            strcpy(message_to_client, result);
                            n = write(newsock_fd, message_to_client, strlen(message_to_client) + 1);
                            read_write_error_check(n, newsock_fd, client_count);
                        }
                        else if(strcmp(command_to_execute, READ) == 0)
                        {
                            printf("Processing %s\n", master_command);
                            char* result = (char*)malloc(READ_SIZE);;
                            char cmd[10];
                            char filename[100];
                            int s_idx;
                            int e_idx;
                            
                            bzero(result, READ_SIZE);
                            int var = sscanf(master_command, "%s %s %d %d", cmd, filename, &s_idx, &e_idx);
                            if(is_file_exists(filename) && (is_owner(filename, client_number) || (is_collaborator(filename, client_number) == 2) || (is_collaborator(filename, client_number) == 1)))
                            {
                                char *file_location;
                                file_location = (char*)malloc(BUFFER_SIZE);
                                strcpy(file_location, CLIENTS_DATA);
                                strcat(file_location, filename);
                                
                                int total_lines = calculate_num_of_lines(file_location);
                                if(var == 4)
                                {
                                    if(s_idx >= -(total_lines) && s_idx <= total_lines && e_idx >= -(total_lines) && e_idx <= total_lines)
                                    {
                                        if((s_idx <= 0 && e_idx <= 0) || (s_idx >= 0 && e_idx >= 0))
                                        {
                                            if (s_idx < 0 && e_idx < 0)
                                            {
                                                s_idx = total_lines + s_idx;
                                                e_idx = total_lines + e_idx;
                                            }
                                            result = read_file(file_location, s_idx, e_idx);
                                        }
                                        else
                                        {
                                            result = "ERROR: Index out of Range.\n";
                                        }
                                    }
                                    else
                                    {
                                        result = "ERROR: Index out of Range.\n";
                                    }
                                }
                                else if(var == 3)
                                {
                                    if(s_idx >= -(total_lines) && s_idx <= total_lines)
                                    {
                                        if (s_idx < 0)
                                        {
                                            s_idx = total_lines + s_idx;
                                        }
                                        result = read_file(file_location, s_idx, MAX_INT);
                                    }
                                    else
                                    {
                                        result = "ERROR: Index out of Range.\n";
                                    }
                                }
                                else
                                {
                                    printf("Reading all the contents of the file.\n");
                                    FILE* fp = fopen(file_location, "r");
                                    if(fp == NULL)
                                    {
                                        error("ERROR: FAILED to read the file.");
                                    }
                                    char line_buffer[BUFFER_SIZE];
                                    while(fgets(line_buffer, BUFFER_SIZE, fp))
                                    {
                                        strcat(result, line_buffer);
                                    }
                                    fclose(fp);
                                }
                            }
                            else
                            {
                                result = "The file doesn't exist or you do not have the permission to delete the file.\n";
                            }
                            n = write(newsock_fd, result, READ_SIZE);
                            read_write_error_check(n, newsock_fd, client_count);
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
    if (fp == NULL)
    {
        error("ERROR: FAILED to reading the file.\n");
    }
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

struct COLLABORATOR *parse_collaborator_file(int num_of_lines)
{
    char collaborator_buffer[BUFFER_SIZE];
    struct COLLABORATOR *collaborator_list = malloc(sizeof(struct COLLABORATOR) * num_of_lines);
    
    FILE *fp = fopen(COLLABORATORS_FILE, "r");
    if (fp == NULL)
    {
        error("ERROR: FAILED to open clients file.");
    }
    for(int i = 0; i < num_of_lines && fscanf(fp, "%[^\n]\n", collaborator_buffer) != EOF; i++)
    {
        int n = sscanf(collaborator_buffer, "%s\t%d\t%s\n",
                       collaborator_list[i].filename,
                       &collaborator_list[i].client_id,
                       collaborator_list[i].permission
                       );
    }
    fclose(fp);
    return collaborator_list;
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

int is_collaborator(char* filename, int client_number)
{
    int num_of_lines = calculate_num_of_lines(COLLABORATORS_FILE);
    struct COLLABORATOR* collaborator = parse_collaborator_file(num_of_lines);
    
    for(int i = 0; i < num_of_lines; i++)
    {
        if(strcmp(collaborator[i].filename, filename) == 0 && collaborator[i].client_id == client_number)
        {
            if(strcmp(collaborator[i].permission, "E") == 0)
            {
                return 2;
            }
            else if(strcmp(collaborator[i].permission, "V") == 0)
            {
                return 1;
            }
        }
    }
    return 0;
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

void check_invitations(int sock_fd, int *client_count, int client_id)
{
    int n;
    char message_to_client[BUFFER_SIZE], message_from_client[BUFFER_SIZE];
    if(has_invitation(sock_fd))
    {
        printf("Invitations Found.\n");
        bzero(message_to_client, BUFFER_SIZE);
        strcpy(message_to_client, "invited");
        n = write(sock_fd, message_to_client, strlen(message_to_client) + 1);
        read_write_error_check(n, sock_fd, client_count);
        
        //send the pending invitations
        char* invitations = get_invitations(sock_fd);
        
        printf("Invitations: %s\n", invitations);
        bzero(message_to_client, BUFFER_SIZE);
        strcpy(message_to_client, invitations);
        n = write(sock_fd, message_to_client, strlen(message_to_client) + 1);
        read_write_error_check(n, sock_fd, client_count);
        free(invitations);
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

void activate_invitation(int sock_fd, int client_id)
{
    int num_of_lines = calculate_num_of_lines(INVITATION_FILE);
    struct INVITATION* invitation_list = parse_invitation_file(num_of_lines);
    
    FILE* fp = fopen(COLLABORATORS_FILE, "a+");
    FILE* fp1 = fopen(INVITATION_FILE, "w");
    if (fp == NULL)
    {
        error("ERROR: FAILED to open collaboratos file.");
    }
    if (fp1 == NULL)
    {
        error("ERROR: FAILED to open invitations file.");
    }
    for(int i = 0; i < num_of_lines; i++)
    {
        if(sock_fd == invitation_list[i].sockfd && invitation_list[i].is_active == 1)
        {
            fprintf(fp, "%s\t%d\t%s\n", invitation_list[i].filename, client_id, invitation_list[i].permission);
            invitation_list[i].is_active = 0;
        }
        fprintf(fp1, "%d\t%d\t%d\t%s\t%s\t%d\n", invitation_list[i].sockfd, invitation_list[i].from_port, invitation_list[i].to_port, invitation_list[i].filename, invitation_list[i].permission, invitation_list[i].is_active);
    }
    fclose(fp);
    fclose(fp1);
}

void turn_off_invitation(int sock_fd)
{
    int num_of_lines = calculate_num_of_lines(INVITATION_FILE);
    struct INVITATION* invitation_list = parse_invitation_file(num_of_lines);
    
    FILE* fp = fopen(INVITATION_FILE, "w");
    if (fp == NULL)
    {
        error("ERROR: FAILED to open invitations file.");
    }
    for(int i = 0; i < num_of_lines; i++)
    {
        if(sock_fd == invitation_list[i].sockfd && invitation_list[i].is_active == 1)
        {
            invitation_list[i].is_active = 0;
        }
        fprintf(fp, "%d\t%d\t%d\t%s\t%s\t%d\n", invitation_list[i].sockfd, invitation_list[i].from_port, invitation_list[i].to_port, invitation_list[i].filename, invitation_list[i].permission, invitation_list[i].is_active);
    }
    fclose(fp);
}

char* get_invitations(int sock_fd)
{
    int num_of_lines = calculate_num_of_lines(INVITATION_FILE);
    struct INVITATION* invitation_list = parse_invitation_file(num_of_lines);
    
    char* invitations;
    invitations = (char*) malloc((BUFFER_SIZE)*sizeof(char));
    strcpy(invitations, "");
    for(int i = 0; i < num_of_lines; i++)
    {
        if(sock_fd == invitation_list[i].sockfd && invitation_list[i].is_active == 1)
        {
            strcat(invitations, invitation_list[i].filename);
            strcat(invitations, "\t");
            strcat(invitations, invitation_list[i].permission);
            strcat(invitations, "\n");
        }
    }
    return invitations;
}

char* get_files()
{
    char *result = (char*)malloc(3000 * sizeof(char));
    char *buffer1 = (char*)malloc(100 * sizeof(char));
    char *buffer2 = (char*)malloc(100 * sizeof(char));
    int filemeta_lines = calculate_num_of_lines(FILES_METADATA);
    struct FILE_METADATA* file_meta = parse_filemeta_file(filemeta_lines);
    int collaborator_lines = calculate_num_of_lines(COLLABORATORS_FILE);
    struct COLLABORATOR* collaborator = parse_collaborator_file(collaborator_lines);
    
    strcpy(result, "");
    for(int i = 0; i < filemeta_lines; i++)
    {
        char *file_location;
        file_location = (char*)malloc(BUFFER_SIZE);
        strcpy(file_location, CLIENTS_DATA);
        strcat(file_location, file_meta[i].filename);
        
        sprintf(buffer1, "Filename:\t%s\nOwner:\t%d\tNo. of lines:\t%d\nCollaborators:\n", file_meta[i].filename, file_meta[i].owner, calculate_num_of_lines(file_location));
        strcat(result, buffer1);
        if(collaborator_lines == 0)
        {
            strcat(result, "NONE\n");
        }
        else
        {
            for(int j = 0; j < collaborator_lines; j++)
            {
                if(strcmp(file_meta[i].filename, collaborator[j].filename) == 0)
                {
                    sprintf(buffer2, "%d\t%s\n", collaborator[j].client_id, collaborator[j].permission);
                    strcat(result, buffer2);
                }
            }
        }
        printf("\n");
        strcat(result, "\n");
    }
    
    return result;
    
}

void copy_backup_to_master(char* masterfile, char* backfile)
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

void insert_at_top(char* file_location, char* message)
{
    char *bkp_location;
    bkp_location = (char*)malloc(BUFFER_SIZE);
    strcpy(bkp_location, CLIENTS_DATA);
    strcpy(bkp_location, "bkp.txt");
    
    char *content;
    content = message;
    FILE *fp, *bkp_fp;
    
    fp = fopen(file_location, "r");
    bkp_fp = fopen(bkp_location, "w");
    
    if (fp == NULL)
    {
        error("ERROR: FAILED to open master file file.");
    }
    
    if (bkp_fp == NULL)
    {
        error("ERROR: FAILED to open backup file.");
    }
    
    message++; //removing the first character i.t "
    message[strlen(message) - 2] = '\0'; // removing the last character i.t "
    fprintf(bkp_fp, "%s\n", message);
    fseek(bkp_fp, 0, SEEK_END);
    
    char line[250];
    while(fscanf(fp, "%[^\n] ", line) != EOF)
    {
        fprintf(bkp_fp, "%s\n", line);
    }
    
    fclose(fp);
    fclose(bkp_fp);
    
    copy_backup_to_master(file_location, bkp_location);
}

void insert_at_index(char* file_location, int index, char* message)
{
    char *bkp_location;
    bkp_location = (char*)malloc(BUFFER_SIZE);
    strcpy(bkp_location, CLIENTS_DATA);
    strcpy(bkp_location, "bkp.txt");
    
    FILE *fp, *bkp_fp;
    char c;
    int newlines = 0;
    
    fp = fopen(file_location, "r");
    bkp_fp = fopen(bkp_location, "w");
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
    
    message++; //removing the first character i.t "
    message[strlen(message) - 2] = '\0'; // removing the last character i.t "
    fprintf(bkp_fp, "%s\n", message);
    
    char line[250];
    while(fscanf(fp, "%[^\n] ", line) != EOF)
    {
        fprintf(bkp_fp, "%s\n", line);
    }
    
    fclose(bkp_fp);
    fclose(fp);
    
    copy_backup_to_master(file_location, bkp_location);
}

void append_file(char* file_location, char* message)
{
    FILE *fp;
    fp = fopen(file_location, "a");
    
    if(fp == NULL)
    {
        error("ERROR: FAILED to open input file.");
    }
    
    message++; //removing the first character i.t "
    message[strlen(message) - 2] = '\0'; // removing the last character i.t "
    fprintf(fp, "%s\n", message);
    fclose(fp);
}

char* insert_in_file(char* filename, char* index, char* message)
{
    char* result;
    char *file_location;
    file_location = (char*)malloc(BUFFER_SIZE);
    strcpy(file_location, CLIENTS_DATA);
    strcat(file_location, filename);
    
    
    int total_lines = calculate_num_of_lines(file_location);
    if(strcmp(index, "X") == 0)
    {
        append_file(file_location, message);
        result = "INSERT sucessfull at the end.\n";
    }
    else
    {
        int line_number = atoi(index);
        if(line_number >= -(total_lines) && line_number <= total_lines)
        {
            if (line_number < 0)
            {
                //Convert -index to positive index
                //-1 -> (num of lines - 1)      249
                //-2 -> (num of lines - 2)      248
                //
                //-250->(num of lines - 250)    0 first line
                line_number = total_lines + line_number;
            }
            
            if (line_number == 0) //add at the top
            {
                insert_at_top(file_location, message);
                result = "INSERT sucessfull at the top.\n";
                printf("INSERT sucessfull at the top.\n");
            }
            else if (line_number == total_lines) // append at the end
            {
                append_file(file_location, message);
                result = "INSERT sucessfull at the end.\n";
                printf("INSERT sucessfull at the end.\n");
            }
            else // add in the middle
            {
                insert_at_index(file_location, line_number, message);
                result = "INSERT sucessfull at the index.\n";
                printf("INSERT sucessfull at the index.\n");
            }
        }
        else
        {
            result = "ERROR: Index out of Range.\n";
            printf("ERROR: Index out of Range.\n"); //TODO: return this to user
        }
    }
    return result;
}

void display_file_content(char* filename)
{
    char *file_location;
    file_location = (char*)malloc(BUFFER_SIZE);
    strcpy(file_location, CLIENTS_DATA);
    strcat(file_location, filename);
    
    
     char line_buffer[BUFFER_SIZE];
     
     FILE *fp;
     fp = fopen(file_location, "r");
    
     if(fp == NULL)
     {
         error("ERROR: FAILED to read file.");
     }
     
     while(fgets(line_buffer, BUFFER_SIZE, fp))
     {
         printf("%s",line_buffer);
     }
     printf("\n");
}

int is_file_exists(char* filename)
{
    int filemeta_lines = calculate_num_of_lines(FILES_METADATA);
    struct FILE_METADATA* file_meta = parse_filemeta_file(filemeta_lines);
    
    for(int i = 0; i < filemeta_lines; i++)
    {
        if(strcmp(file_meta[i].filename, filename) == 0)
        {
            return 1;
        }
    }
    return 0;
}

char* delete(char* file_location, int s_index, int e_index)
{
    char* result;
    int small, large;
    if(s_index < e_index)
    {
        small = s_index;
        large = e_index;
    }
    else
    {
        large = s_index;
        small = e_index;
    }
    char *bkp_location;
    bkp_location = (char*)malloc(BUFFER_SIZE);
    strcpy(bkp_location, CLIENTS_DATA);
    strcat(bkp_location, "bkp.txt");
    
    FILE *fp, *bkp_fp;
    
    fp = fopen(file_location, "r");
    bkp_fp = fopen(bkp_location, "w");
    
    int idx = 0;
    char line[250];
    while(fscanf(fp, "%[^\n] ", line) != EOF)
    {
        if(e_index == MAX_INT)
        {
            if(idx != s_index)
            {
                fprintf(bkp_fp, "%s\n", line);
            }
        }
        else
        {
            if(idx < small || idx > large)
            {
                fprintf(bkp_fp, "%s\n", line);
            }
        }
        idx++;
    }
    
    fclose(fp);
    fclose(bkp_fp);
    
    copy_backup_to_master(file_location, bkp_location);
    result = "Delete Successful.\n";
    return result;
}

char* read_file(char* file_location, int s_index, int e_index)
{
    char* result =(char*)malloc(READ_SIZE);
    int small, large;
    if(s_index < e_index)
    {
        small = s_index;
        large = e_index;
    }
    else
    {
        large = s_index;
        small = e_index;
    }
    
    FILE *fp;
    fp = fopen(file_location, "r");
    
    int idx = 0;
    char line[250];
    bzero(result, READ_SIZE);
    while(fscanf(fp, "%[^\n] ", line) != EOF)
    {
        if(e_index == MAX_INT)
        {
            if(idx == s_index)
            {
                strcat(result, line);
                strcat(result, "\n");
                break;
            }
        }
        else
        {
            if(idx >= small && idx <= large)
            {
                strcat(result, line);
                strcat(result, "\n");
            }
        }
        idx++;
    }
    
    fclose(fp);
    return result;
    
}
