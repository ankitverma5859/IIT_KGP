#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <ctype.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <regex.h>
#include <sys/stat.h>
    
#define TRUE 1
#define FALSE 0
#define PORT 8888

const int BUFFER_SIZE = 256;
const int MAX_CLIENTS = 5;

char *REG_EXIT = "^/exit\n$";
char *REG_SORT = "^/sort [a-zA-Z0-9_]*.txt (date|item_name|price)\n$";
char *REG_MERGE = "^/merge [a-zA-Z0-9_]*.txt [a-zA-Z0-9_]*.txt [a-zA-Z0-9_]*.txt (date|item_name|price)\n$";
char *REG_SIMILARITY = "^/similarity [a-zA-Z0-9_]*.txt [a-zA-Z0-9_]*.txt\n$";
char *REG_DATALINE = "^(0[1-9]|[12][0-9]|3[01]).(0[1-9]|1[0-2]).[0-9]{4}\t[a-zA-Z0-9_ -]*\t[0-9]*.[0-9]{2}\n";

char *EXIT = "exit";
char *SORT = "sort";
char *MERGE = "merge";
char *SIMILARITY = "similarity";
char *DATE = "date\n";
char *ITEM_NAME = "item_name\n";
char *PRICE = "price\n";
char *NONE = "none";
char *BASE_LOCATION = "server_data/";
char *FILE_SIZE = "filesize";
char *SEND_FILE = "sendfile";
char *FILE_1 = "file_1.txt";
char *FILE_2 = "file_2.txt";
char *SORTED_FILE_1 = "sortedfile_1.txt";

struct DATE
{
    int year;
    int month;
    int day;
};

struct BILL
{
    struct DATE bill_date;
    char item_name[BUFFER_SIZE];
    float price;
};

void error(char *msg);
void signal_handler(int sig);
void read_write_error_check(int n, int *client_count, int sock_fd);
long convert_string_to_long(char *size);
char* toLower(char* s);
int compare_date(struct DATE date1, struct DATE date2);
void preprocess(char *filename);
int calculate_num_of_lines(char* filelocation);
char* create_file(char *date, char *filename);
int regex_match(char *string, char *regex_string);
char* validate_command(char *buffer);
int validate_file(char *filename);
struct BILL *parse_file(char* filelocation, int num_of_lines);
void copy_bill(struct BILL* left, struct BILL* right, int left_index, int right_index);
void merge(struct BILL* parsed_bill, int l, int m, int r, char* sorting_criteria);
void merge_sort(struct BILL* parsed_bill, int l, int r, char* sorting_criteria);
void sort_bill(struct BILL* parsed_bill, int num_of_lines, char* message_from_client);
char* convert_struct_to_string(struct BILL* parsed_bill, int num_of_lines);
long long stat_filesize(char *filename);
void find_similarity(struct BILL* parsed_bill_1, struct BILL* parsed_bill_2, int len1, int len2);
int compare_with_sorted(struct BILL* parsed_bill, struct BILL* parsed_bill_sorted, char* sorting_criteria, int num_of_lines);
struct BILL *merge_bills(struct BILL* parsed_bill1, struct BILL* parsed_bill2, int l1, int l2, char* sorting_criteria);
void swap(char *x, char *y);
char* reverse(char *buffer, int i, int j);
char* itoa(int value, char* buffer, int base);


/*
 Main function
*/
int main(int argc , char *argv[])
{
    signal(SIGINT, signal_handler);
    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[5] ,
        max_clients = 5 , activity, i , valread , sd;
    int max_sd;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];
    char message_to_client[BUFFER_SIZE], message_from_client[BUFFER_SIZE];
        
    fd_set readfds;
        
    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }
        
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
        sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
        
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);
        
    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
        
    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");
        
    while(TRUE)
    {
        FD_ZERO(&readfds);
    
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
            
        for ( i = 0 ; i < max_clients ; i++)
        {
            sd = client_socket[i];
                
            if(sd > 0)
                FD_SET( sd , &readfds);
                
            if(sd > max_sd)
                max_sd = sd;
        }
    
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
    
        if ((activity < 0) && (errno!=EINTR))
        {
            printf("select error");
        }
            
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket,
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs
                (address.sin_port));
            
            int n;
            while(1)
            {
                bzero(message_from_client, BUFFER_SIZE);
                n = read(new_socket, message_from_client, BUFFER_SIZE);
                
                char master_command[BUFFER_SIZE];
                strcpy(master_command, message_from_client);
                char* sorting_criteria;
                
                char *command_to_execute = validate_command(message_from_client);
                
                // Clients asks to disconnect: /exit
                if(strcmp(command_to_execute, EXIT) == 0)
                {
                    printf("Disconnected from %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    break;
                }
                else
                {
                    if(strcmp(command_to_execute, SORT) == 0)
                    {
                        //STEP 1: Ask for the filesize
                        strcpy(message_to_client, FILE_SIZE);
                        n = write(new_socket, message_to_client, strlen(message_to_client) + 1);
                        //read_write_error_check(n, client_count, new_socket);
                        
                        //STEP 2: READ the filesize from client
                        bzero(message_from_client, BUFFER_SIZE);
                        n = read(new_socket, message_from_client, BUFFER_SIZE);
                        //read_write_error_check(n, client_count, new_socket);
                        
                        //STEP 3: Convert the string filesize to long
                        long filesize = convert_string_to_long(message_from_client);
                        
                        //STEP 4: Ask Client to send the file
                        strcpy(message_to_client, SEND_FILE);
                        n = write(new_socket, message_to_client, strlen(message_to_client) + 1);
                        //read_write_error_check(n, client_count, new_socket);
                        
                        //STEP 5: READ file contents from the buffer
                        char* data = (char*)malloc ((filesize+1)*sizeof(char));
                        n = read(new_socket, data, filesize);
                        
                        //read_write_error_check(n, client_count, new_socket);
                        
                        //STEP 6: Create a file for the buffer
                        char* file_location = create_file(data, FILE_1);
                        free(data);
                        
                        //STEP 7: Validate the contents of file with REGEX
                        if(validate_file(file_location))
                        {
                            bzero(message_from_client, BUFFER_SIZE);
                            
                            //STEP 9: Create a data structure for file content
                            int num_of_lines = calculate_num_of_lines(file_location);
                            struct BILL* parsed_bill = parse_file(file_location, num_of_lines);
                            
                            //STEP 10: Sort the content
                            strtok(master_command, " ");
                            strtok(NULL, " ");
                            sorting_criteria = strtok(NULL, " ");
                            sort_bill(parsed_bill, num_of_lines, sorting_criteria);
                            
                            // Data
                            data = convert_struct_to_string(parsed_bill, num_of_lines);
                            
                            //STEP : Send filesize to client
                            long filesize = strlen(data) + 1;
                            char fsize[256];
                            sprintf(fsize, "%ld", filesize);
                            n = write(new_socket, fsize, strlen(fsize));
                            
                            //Receive request for file
                            n = read(new_socket, message_from_client, BUFFER_SIZE);
                            
                            //Send the sorted content to client
                            n = write(new_socket, data, filesize);
                            
                            //Free Memory
                            free(parsed_bill);
                        }
                        else
                        {
                            //STEP 8: Send error if content not in standard format
                            strcpy(message_to_client, "INVALID_FILE_CONTENT\n");
                            n = write(new_socket, message_to_client, strlen(message_to_client) + 1);
                            //read_write_error_check(n, client_count, new_socket);
                        }
                    }
                    else if(strcmp(command_to_execute, MERGE) == 0)
                    {
                        //S2: Server received Merge command and now in merge block;
                        
                        //S3: Ask for the filesize
                        strcpy(message_to_client, FILE_SIZE);
                        n = write(new_socket, message_to_client, strlen(message_to_client) + 1);
                        //read_write_error_check(n, client_count, new_socket);
                        
                        //S6 Get filesize of f1
                        bzero(message_from_client, BUFFER_SIZE);
                        n = read(new_socket, message_from_client, BUFFER_SIZE);
                        //read_write_error_check(n, client_count, new_socket);
                        long filesize_1 = convert_string_to_long(message_from_client);
                        printf("Filesize1: %ld\n", filesize_1);
                        
                        //S7: Ask Client to send the file
                        strcpy(message_to_client, SEND_FILE);
                        n = write(new_socket, message_to_client, strlen(message_to_client) + 1);
                        //read_write_error_check(n, client_count, new_socket);
                        
                        //S9: Receive file1
                        char* data1 = (char*)malloc ((filesize_1+1)*sizeof(char));
                        n = read(new_socket, data1, filesize_1);
                        //read_write_error_check(n, client_count, new_socket);
                        
                        //S10: Create a file for the buffer
                        char* file_location1 = create_file(data1, FILE_1);
                        free(data1);
                         
                        //S11: Ask for the filesize of f2
                        strcpy(message_to_client, FILE_SIZE);
                        n = write(new_socket, message_to_client, strlen(message_to_client) + 1);
                        //read_write_error_check(n, client_count, new_socket);
                        
                        //S13 Get filesize of f2
                        bzero(message_from_client, BUFFER_SIZE);
                        n = read(new_socket, message_from_client, BUFFER_SIZE);
                        //read_write_error_check(n, client_count, new_socket);
                        long filesize_2 = convert_string_to_long(message_from_client);
                        printf("Filesize2: %ld\n", filesize_2);
                        
                        //S14: Ask Client to send the file2
                        strcpy(message_to_client, SEND_FILE);
                        n = write(new_socket, message_to_client, strlen(message_to_client) + 1);
                        //read_write_error_check(n, client_count, new_socket);
                        
                        //S16: Receive file2
                        char* data2 = (char*)malloc ((filesize_2+1)*sizeof(char));
                        n = read(new_socket, data2, filesize_2);
                        //read_write_error_check(n, client_count, new_socket);
                        
                        //S17: Create a file for the buffer
                        char* file_location2 = create_file(data2, FILE_2);
                        free(data2);

                        //Create response
                        //Validation of content
                        if(validate_file(file_location1) && validate_file(file_location2))
                        {
                            //Load as structure
                            int num_of_lines1 = calculate_num_of_lines(file_location1);
                            struct BILL* parsed_bill1 = parse_file(file_location1, num_of_lines1);
                            struct BILL* parsed_bill_sorted1 = parse_file(file_location1, num_of_lines1);
                            
                            int num_of_lines2 = calculate_num_of_lines(file_location2);
                            struct BILL* parsed_bill2 = parse_file(file_location2, num_of_lines2);
                            struct BILL* parsed_bill_sorted2 = parse_file(file_location2, num_of_lines2);
                            
                            char* sorting_criteria;
                            strtok(master_command, " ");
                            strtok(NULL, " ");
                            strtok(NULL, " ");
                            strtok(NULL, " ");
                            sorting_criteria = strtok(NULL, " ");
                            printf("Sorting Criteria: %s", sorting_criteria);
                            
                            //Sorting
                            sort_bill(parsed_bill_sorted1, num_of_lines1, sorting_criteria);
                            sort_bill(parsed_bill_sorted2, num_of_lines2, sorting_criteria);
                            
                            //Compare with sorted
                            int cmp_res1 = compare_with_sorted(parsed_bill1, parsed_bill_sorted1, sorting_criteria, num_of_lines1);
                            int cmp_res2 = compare_with_sorted(parsed_bill2, parsed_bill_sorted2, sorting_criteria, num_of_lines2);
                            
                            if((cmp_res1 == 1) && (cmp_res2 == 1))
                            {
                                struct BILL* parsed_bill = merge_bills(parsed_bill1, parsed_bill2, num_of_lines1, num_of_lines2, sorting_criteria);
                                printf("Merged Bills: \n");
                                for(int i = 0; i < (num_of_lines1 + num_of_lines2); i ++)
                                {
                                    printf("%d.%d.%d\t%s\t%.2f\n",
                                           parsed_bill[i].bill_date.day,
                                           parsed_bill[i].bill_date.month,
                                           parsed_bill[i].bill_date.year,
                                           parsed_bill[i].item_name,
                                           parsed_bill[i].price
                                           );
                                }
                                
                                // Data
                                //char* data = convert_struct_to_string(parsed_bill, num_of_lines1 + num_of_lines2);
                                
                                //printf("S: \n%s\n", data);
                                //STEP : Send filesize to client
                                //long filesize = strlen(data) + 1;
                                //char fsize[256];
                                //sprintf(fsize, "%ld", filesize);
                                //n = write(new_socket, fsize, strlen(fsize));
                                
                                //Receive request for file
                                //n = read(new_socket, message_from_client, BUFFER_SIZE);
                                
                                //Send the sorted content to client
                                //n = write(new_socket, data, filesize);
                                
                                
                            }
                            else
                            {
                                printf("Hello\n");
                                //bzero(message_from_client, BUFFER_SIZE);
                                //strcpy(message_to_client, "SERVER: FILES are not sorted.\n");
                                //n = write(new_socket, message_to_client, strlen(message_to_client) + 1);
                                //read_write_error_check(n, client_count, new_socket);
                            }
                        }
                        
                        bzero(message_from_client, BUFFER_SIZE);
                        strcpy(message_to_client, "SERVER: MERGE Completed.\n");
                        n = write(new_socket, message_to_client, strlen(message_to_client) + 1);
                        //read_write_error_check(n, client_count, new_socket);
                         
                    }
                    else if(strcmp(command_to_execute, SIMILARITY) == 0)
                    {
                        // Extract the two filenames from command
                        strtok(master_command, " ");
                        char* filename1 = strtok(NULL, " ");
                        char* filename2 = strtok(NULL, " ");
                        
                        // READ the filesize of file1 from client
                        bzero(message_from_client, BUFFER_SIZE);
                        n = read(new_socket, message_from_client, BUFFER_SIZE);
                        //read_write_error_check(n, client_count, new_socket);
                        
                        // Convert the string filesize to long
                        long filesize_1 = convert_string_to_long(message_from_client);
                        
                        // Ask Client to send the file_1
                        strcpy(message_to_client, SEND_FILE);
                        n = write(new_socket, message_to_client, strlen(message_to_client) + 1);
                        //read_write_error_check(n, client_count, new_socket);
                        
                        // READ file_1 contents from the buffer
                        char* data = (char*)malloc ((filesize_1+1)*sizeof(char));
                        n = read(new_socket, data, filesize_1);
                        //read_write_error_check(n, client_count, new_socket);
                        
                        // Create a file for the buffer
                        char* file_location_1 = create_file(data, FILE_1);
                        free(data);
                        
                        if(validate_file(file_location_1))
                        {
                            bzero(message_from_client, BUFFER_SIZE);
                            
                            // Ask for the filesize of file_1
                            strcpy(message_to_client, FILE_SIZE);
                            n = write(new_socket, message_to_client, strlen(message_to_client) + 1);
                            //read_write_error_check(n, client_count, new_socket);
                            
                            // READ the filesize from client of file_2
                            bzero(message_from_client, BUFFER_SIZE);
                            n = read(new_socket, message_from_client, BUFFER_SIZE);
                            //read_write_error_check(n, client_count, new_socket);
                            
                            // Convert the string filesize to long of file_2
                            long filesize_2 = convert_string_to_long(message_from_client);
                            //printf("Filesize2: %ld", filesize_2);
                            
                            // Ask Client to send the file of file_2
                            strcpy(message_to_client, SEND_FILE);
                            n = write(new_socket, message_to_client, strlen(message_to_client) + 1);
                            //read_write_error_check(n, client_count, new_socket);
                            
                            // READ file contents from the buffer of file_2
                            char* data = (char*)malloc ((filesize_2+1)*sizeof(char));
                            n = read(new_socket, data, filesize_2);
                            //read_write_error_check(n, client_count, new_socket);
                            
                            //STEP 6: Create a file_2 for the buffer
                            char* file_location_2 = create_file(data, FILE_2);
                            free(data);
                            
                            if(validate_file(file_location_2))
                            {
                                bzero(message_from_client, BUFFER_SIZE);
                                
                                // Create a data structure for file content
                                int num_of_lines_1 = calculate_num_of_lines(file_location_1);
                                struct BILL* parsed_bill_1 = parse_file(file_location_1, num_of_lines_1);
                                
                                int num_of_lines_2 = calculate_num_of_lines(file_location_2);
                                struct BILL* parsed_bill_2 = parse_file(file_location_2, num_of_lines_2);
                                
                                // Find similarity
                                find_similarity(parsed_bill_1, parsed_bill_2, num_of_lines_1, num_of_lines_2);
                                
                                strcpy(message_to_client, "File two with validated content\n");
                                n = write(new_socket, message_to_client, strlen(message_to_client) + 1);
                                //read_write_error_check(n, client_count, new_socket);
                            }
                            else
                            {
                                //STEP 8: Send error if content not in standard format
                                strcpy(message_to_client, "INVALID_FILE_CONTENT\n");
                                n = write(new_socket, message_to_client, strlen(message_to_client) + 1);
                                //read_write_error_check(n, client_count, new_socket);
                            }
                        }
                        else
                        {
                            //STEP 8: Send error if content not in standard format
                            strcpy(message_to_client, "INVALID_FILE_CONTENT\n");
                            n = write(new_socket, message_to_client, strlen(message_to_client) + 1);
                            //read_write_error_check(n, client_count, new_socket);
                        }
                    }
                    bzero(message_from_client, BUFFER_SIZE);
                    bzero(message_to_client, BUFFER_SIZE);
                }
            }
        
            
            
            
            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++)
            {
                //if position is empty
                if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n" , i);
                        
                    break;
                }
            }
        }
            
        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];
                
            if (FD_ISSET( sd , &readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read( sd , buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" ,
                        inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
                        
                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                }
                    
                //Echo back the message that came in
                else
                {
                    //set the string terminating NULL byte on the end
                    //of the data read
                    buffer[valread] = '\0';
                    send(sd , buffer , strlen(buffer) , 0 );
                }
            }
        }
    }
        
    return 0;
}

struct BILL *merge_bills(struct BILL* parsed_bill1, struct BILL* parsed_bill2, int l1, int l2, char* sorting_criteria)
{
    printf("Merging Bills.\n");
    int len = l1 < l2 ? l2 : l1;
    struct BILL *parsed_bill = malloc(sizeof(struct BILL) * (l1 + l2));
    
    int k = 0;
    int i = 0;
    int j = 0;
    while(i < l1 && j < l2)
    {
        if(strcmp(sorting_criteria, DATE) == 0)
        {
            int res = compare_date(parsed_bill1[i].bill_date, parsed_bill2[j].bill_date);
            if(res <= 0)
            {
                copy_bill(parsed_bill, parsed_bill1, k, i);
                i = i + 1;
            }
            else
            {
                copy_bill(parsed_bill, parsed_bill2, k, j);
                j = j+ 1;
            }
        }
        else if(strcmp(sorting_criteria, ITEM_NAME) == 0)
        {
            char left[BUFFER_SIZE];
            char right[BUFFER_SIZE];
            strcpy(left, parsed_bill1[i].item_name);
            strcpy(right, parsed_bill2[j].item_name);
            int res = strcmp(toLower(left), toLower(right));
            if(res <= 0)
            {
                copy_bill(parsed_bill, parsed_bill1, k, i);
                i = i + 1;
            }
            else
            {
                copy_bill(parsed_bill, parsed_bill2, k, j);
                j = j+ 1;
            }
        }
        else if(strcmp(sorting_criteria, PRICE) == 0)
        {
            if (parsed_bill1[i].price <= parsed_bill1[j].price)
            {
                copy_bill(parsed_bill, parsed_bill1, k, i);
                i = i + 1;
            }
            else {
                copy_bill(parsed_bill, parsed_bill2, k, j);
                j = j + 1;
            }
        }
        k= k + 1;
    }
    
    while(i < l1)
    {
        copy_bill(parsed_bill, parsed_bill1, k, i);
        i = i + 1;
        k = k + 1;
    }
    
    while(j < l2)
    {
        copy_bill(parsed_bill, parsed_bill2, k, j);
        j = j + 1;
        k = k + 1;
    }
    
    return parsed_bill;
}

int compare_with_sorted(struct BILL* parsed_bill, struct BILL* parsed_bill_sorted, char* sorting_criteria, int num_of_lines)
{
    for(int i = 0; i < num_of_lines; i++)
    {
        if(strcmp(sorting_criteria, DATE) == 0)
        {
            if(
               (parsed_bill[i].bill_date.day != parsed_bill_sorted[i].bill_date.day) ||
               (parsed_bill[i].bill_date.month != parsed_bill_sorted[i].bill_date.month) ||
               (parsed_bill[i].bill_date.year != parsed_bill_sorted[i].bill_date.year)
               )
            {
                return 0;
            }
        }
        else if(strcmp(sorting_criteria, ITEM_NAME) == 0)
        {
            if(parsed_bill[i].item_name != parsed_bill_sorted[i].item_name)
            {
                return 0;
            }
        }
        else if(strcmp(sorting_criteria, PRICE) == 0)
        {
            if(parsed_bill[i].price != parsed_bill_sorted[i].price)
            {
                return 0;
            }
        }
    }
    return 1;
}

void find_similarity(struct BILL* parsed_bill_1, struct BILL* parsed_bill_2, int len1, int len2)
{
    printf("Bill1:\n");
    for(int i = 0; i < len1; i++)
    {
        for (int j = 0; j < len2; j++)
        {
            //printf("%s\t%s\n",parsed_bill_1[i].item_name, parsed_bill_2[i].item_name);
          if(
             strcmp(parsed_bill_1[i].item_name, parsed_bill_2[j].item_name) == 0 ||
             parsed_bill_1[i].price == parsed_bill_2[j].price ||
             compare_date(parsed_bill_1[i].bill_date, parsed_bill_2[i].bill_date) == 0
             )
          {
              printf("%d.%d.%d\t%s\t%.2f\t|\t%d.%d.%d\t%s\t%.2f\n",
                     parsed_bill_1[i].bill_date.day,
                     parsed_bill_1[i].bill_date.month,
                     parsed_bill_1[i].bill_date.year,
                     parsed_bill_1[i].item_name,
                     parsed_bill_1[i].price,
                     parsed_bill_2[j].bill_date.day,
                     parsed_bill_2[j].bill_date.month,
                     parsed_bill_2[j].bill_date.year,
                     parsed_bill_2[j].item_name,
                     parsed_bill_2[j].price
                     );
          }
        }
    }
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
void read_write_error_check(int n, int *client_count, int sock_fd)
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

char* toLower(char* s)
{
  for(char *p=s; *p; p++)
      *p=tolower(*p);
  return s;
}

int compare_date(struct DATE date1, struct DATE date2)
{
    int year1 = date1.year;
    int year2 = date2.year;
    int month1 = date1.month;
    int month2 = date2.month;
    int day1 = date1.day;
    int day2 = date2.day;
    
    int result = 0;
    
    int date1_score = (year1 * 1000) + (month1 * 100) + day1;
    int date2_score = (year2 * 1000) + (month2 * 100) + day2;
    
    if(date1_score <= date2_score)
    {
        result = -1;
    }
    else if(date1_score == date2_score)
    {
        result = 0;
    }
    else
    {
        result = 1;
    }
     
    return result;
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

char* create_file(char *data, char *filename)
{
    char *file_location;
    file_location = (char*)malloc(BUFFER_SIZE);
    strcpy(file_location, BASE_LOCATION);
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

/*
    Validates an input file against the Standard format <date>\t<item>\t<price>
*/
int validate_file(char *filename)
{
    printf("Validating %s\n", filename); // DO NOT REMOVE
    preprocess(filename);
    char line_buffer[BUFFER_SIZE];
    
    FILE *fp;
    fp = fopen(filename, "r");
   
    if(fp == NULL)
    {
        error("ERROR: FAILED to read input file.");
    }
    
    while(fgets(line_buffer, BUFFER_SIZE, fp))
    {
        if(line_buffer[0] != '\n')
        {
            
            int len = strlen(line_buffer);
            char* a = strtok(line_buffer, "\t");
            char* b = strtok(NULL, "\t");
            char* c = strtok(NULL, "");
            printf("A: %s B: %s C: %s", a, b, c);
            
            if(!(regex_match(a, "^[0-9]{1,2}.[0-9]{1,2}.[0-9]{4}$")))
            {
                printf("Invalid Data: %s\n", a);
                return 0;
            }
            
            if(!regex_match(b, "^[a-zA-Z0-9_ -]*$"))
            {
                printf("Invalid Data: %s\n", a);
                return 0;
            }
            
            if(!regex_match(c, "^[0-9]*.[0-9]{1,2}"))
            {
                printf("Invalid Data: %s\n", a);
                return 0;
            }
            
            //return 0;
            /*
            if(!regex_match(line_buffer, REG_DATALINE))
            {
                printf("Incorrect Data: %s\n", line_buffer);
                return 0;
            }
             */
        }
    }
    fclose(fp);
    return 1;
}

struct BILL *parse_file(char* filelocation, int num_of_lines)
{
    char bill_buffer[BUFFER_SIZE];
    //int num_of_lines = calculate_num_of_lines(filelocation);
    struct BILL *parsed_bill = malloc(sizeof(struct BILL) * num_of_lines);
    
    FILE *fp = fopen(filelocation, "r");
    
    for(int i = 0; i < num_of_lines && fscanf(fp, "%[^\n]\n", bill_buffer) != EOF; i++)
    {
        
        int n = sscanf(bill_buffer, "%d.%d.%d\t%[^\t]\t%f",
                       &parsed_bill[i].bill_date.day,
                       &parsed_bill[i].bill_date.month,
                       &parsed_bill[i].bill_date.year,
                       parsed_bill[i].item_name,
                       &parsed_bill[i].price);
    }
    fclose(fp);
    return parsed_bill;
    /*
    for(int i = 0; i < num_of_lines; i++)
    {
        printf("%d.%d.%d\t%s\t%f\n",
               parsed_bill[i].bill_date.day,
               parsed_bill[i].bill_date.month,
               parsed_bill[i].bill_date.year,
               parsed_bill[i].item_name,
               parsed_bill[i].price);
    }
     */
    
}

void copy_bill(struct BILL* left_bill_item, struct BILL* right_bill_item, int left_index, int right_index)
{
    left_bill_item[left_index].bill_date.day = right_bill_item[right_index].bill_date.day;
    left_bill_item[left_index].bill_date.month = right_bill_item[right_index].bill_date.month;
    left_bill_item[left_index].bill_date.year = right_bill_item[right_index].bill_date.year;
    strcpy(left_bill_item[left_index].item_name, right_bill_item[right_index].item_name);
    left_bill_item[left_index].price = right_bill_item[right_index].price;
}

void merge(struct BILL* parsed_bill, int l, int m, int r, char* sorting_criteria)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;
  
    struct BILL left_bill[n1];
    struct BILL right_bill[n2];
  
    for (i = 0; i < n1; i++)
    {
        copy_bill(left_bill, parsed_bill, i, l + i);
    }
        
    for (j = 0; j < n2; j++)
    {
        copy_bill(right_bill, parsed_bill, j, m + 1 + j);
    }
        
    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2)
    {
        if(strcmp(sorting_criteria, DATE) == 0)
        {
            int res = compare_date(left_bill[i].bill_date, right_bill[j].bill_date);
            if(res <= 0)
            {
                copy_bill(parsed_bill, left_bill, k, i);
                i++;
            }
            else {
                copy_bill(parsed_bill, right_bill, k, j);
                j++;
            }
        }
        else if(strcmp(sorting_criteria, PRICE) == 0)
        {
            if (left_bill[i].price <= right_bill[j].price)
            {
                copy_bill(parsed_bill, left_bill, k, i);
                i++;
            }
            else {
                copy_bill(parsed_bill, right_bill, k, j);
                j++;
            }
        }
        else if(strcmp(sorting_criteria, ITEM_NAME) == 0)
        {
            char left[BUFFER_SIZE];
            char right[BUFFER_SIZE];
            strcpy(left, left_bill[i].item_name);
            strcpy(right, right_bill[j].item_name);
            int res = strcmp(toLower(left), toLower(right));
            if(res <= 0)
            {
                copy_bill(parsed_bill, left_bill, k, i);
                i++;
            }
            else {
                copy_bill(parsed_bill, right_bill, k, j);
                j++;
            }
        }
        k++;
    }
  
    while (i < n1) {
        copy_bill(parsed_bill, left_bill, k, i);
        i++;
        k++;
    }
  
    while (j < n2) {
        copy_bill(parsed_bill, right_bill, k, j);
        j++;
        k++;
    }
}

void merge_sort(struct BILL* parsed_bill, int l, int r, char* sorting_criteria)
{
    if (l < r) {
        int m = l + (r - l) / 2;
  
        merge_sort(parsed_bill, l, m, sorting_criteria);
        merge_sort(parsed_bill, m + 1, r, sorting_criteria);
  
        merge(parsed_bill, l, m, r, sorting_criteria);
    }
}

void sort_bill(struct BILL* parsed_bill, int num_of_lines, char* sorting_criteria)
{
    
    printf("Before Sorting: \n");
    for(int i = 0; i < num_of_lines; i++)
    {
        printf("%d.%d.%d\t%s\t%.2f\n",
               parsed_bill[i].bill_date.day,
               parsed_bill[i].bill_date.month,
               parsed_bill[i].bill_date.year,
               parsed_bill[i].item_name,
               parsed_bill[i].price);
    }
    
    if(strcmp(sorting_criteria, DATE) == 0)
    {
        merge_sort(parsed_bill, 0, num_of_lines - 1, DATE);
    }
    else if(strcmp(sorting_criteria, ITEM_NAME) == 0)
    {
        merge_sort(parsed_bill, 0, num_of_lines - 1, ITEM_NAME);
    }
    else if(strcmp(sorting_criteria, PRICE) == 0)
    {
        merge_sort(parsed_bill, 0, num_of_lines - 1, PRICE);
    }
    /*
    printf("After Sorting: \n");
    for(int i = 0; i < num_of_lines; i++)
    {
        printf("%d.%d.%d\t%s\t%.2f\n",
               parsed_bill[i].bill_date.day,
               parsed_bill[i].bill_date.month,
               parsed_bill[i].bill_date.year,
               parsed_bill[i].item_name,
               parsed_bill[i].price);
    }
    */
}


char* convert_struct_to_string(struct BILL* parsed_bill, int num_of_lines)
{
    char* string_bill;
    strcpy(string_bill, "");
    for(int i = 0; i < num_of_lines; i++)
    {
        char buf[100];
        char line[BUFFER_SIZE];
        sprintf(buf, "%d", parsed_bill[i].bill_date.day);
        //itoa(parsed_bill[i].bill_date.day, buf, 10);
        strcpy(line, buf);
        strcat(line, ".");
        sprintf(buf, "%d", parsed_bill[i].bill_date.month);
        strcat(line, buf);
        strcat(line, ".");
        sprintf(buf, "%d", parsed_bill[i].bill_date.year);
        strcat(line, buf);
        strcat(line, "\t");
        strcat(line, parsed_bill[i].item_name);
        strcat(line, "\t");
        sprintf(buf, "%.2f", parsed_bill[i].price);
        strcat(line, buf);
        strcat(line, "\n");
        strcat(string_bill, line);
    }
    strcat(string_bill, "\n");
    return string_bill;
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


// Function to swap two numbers
void swap(char *x, char *y) {
    char t = *x; *x = *y; *y = t;
}
 
// Function to reverse `buffer[i…j]`
char* reverse(char *buffer, int i, int j)
{
    while (i < j) {
        swap(&buffer[i++], &buffer[j--]);
    }
 
    return buffer;
}

// Iterative function to implement `itoa()` function in C
char* itoa(int value, char* buffer, int base)
{
    // invalid input
    if (base < 2 || base > 32) {
        return buffer;
    }
 
    // consider the absolute value of the number
    int n = abs(value);
 
    int i = 0;
    while (n)
    {
        int r = n % base;
 
        if (r >= 10) {
            buffer[i++] = 65 + (r - 10);
        }
        else {
            buffer[i++] = 48 + r;
        }
 
        n = n / base;
    }
 
    // if the number is 0
    if (i == 0) {
        buffer[i++] = '0';
    }
 
    // If the base is 10 and the value is negative, the resulting string
    // is preceded with a minus sign (-)
    // With any other base, value is always considered unsigned
    if (value < 0 && base == 10) {
        buffer[i++] = '-';
    }
 
    buffer[i] = '\0'; // null terminate string
 
    // reverse the string and return it
    return reverse(buffer, 0, i - 1);
}
