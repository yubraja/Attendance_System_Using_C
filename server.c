#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define FILE_NAME_FORMAT "attendance_%s.tsv" // Modified file name format

// Structure to store attendance data
typedef struct
{
    char roll_number[BUFFER_SIZE];
    char ip_address[BUFFER_SIZE];
    char status[2]; // Changed to 2 characters for "p" or "d"
    char timestamp[20];
} AttendanceEntry;

// Function to check if the IP address exists in the file
int isIPDuplicate(const char *file_name, const char *ip_address)
{
    FILE *file = fopen(file_name, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return -1; // Error opening file
    }

    // Read and discard the header line
    char header[BUFFER_SIZE];
    fgets(header, sizeof(header), file);

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), file))
    {
        char ip[BUFFER_SIZE];
        if (sscanf(line, "%*s %s", ip) == 1 && strcmp(ip, ip_address) == 0)
        {
            fclose(file);
            return 1; // IP address found
        }
    }

    fclose(file);
    return 0; // IP address not found
}

// Function to handle client connection
void handle_client(int connfd, const char *file_name)
{
    char buffer[BUFFER_SIZE];

    // Receive roll number from client
    int bytes_received = recv(connfd, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0)
    {
        // If no data received or error, close connection
        close(connfd);
        return;
    }

    // Get client IP address
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    getpeername(connfd, (struct sockaddr *)&cliaddr, &len);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(cliaddr.sin_addr), client_ip, INET_ADDRSTRLEN);

    // Check if IP address exists in the file
    int is_duplicate = isIPDuplicate(file_name, client_ip);

    // Get current timestamp
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    // Write attendance entry to file
    FILE *file = fopen(file_name, "a");
    if (file == NULL)
    {
        perror("Error opening file");
        close(connfd);
        return;
    }

    // Extract roll number from buffer
    buffer[bytes_received] = '\0'; // Null-terminate the buffer
    fprintf(file, "%s\t%s\t%s\t%s\n", buffer, client_ip, (is_duplicate ? "d" : "p"), timestamp);
    fclose(file);

    // Send response to client
    send(connfd, "Attendance Recorded", 20, 0);

    printf("Attendance recorded for roll number %s\n", buffer);

    close(connfd);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    int sockfd, connfd;
    struct sockaddr_in servaddr;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Prepare the server address
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(sockfd, 5) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    // Get the current date
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    char date_buffer[20];
    strftime(date_buffer, sizeof(date_buffer), "%d_%b", local_time); // Format: DD_Mon

    // Create the file name with the current date
    char FILE_NAME[BUFFER_SIZE];
    sprintf(FILE_NAME, FILE_NAME_FORMAT, date_buffer);

    FILE *file = fopen(FILE_NAME, "w");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "Roll Number\tIP Address\tStatus\tTimestamp\n");
    fclose(file);

    while (1)
    {
        // Accept connection
        socklen_t len = sizeof(servaddr);
        connfd = accept(sockfd, (struct sockaddr *)&servaddr, &len);
        if (connfd < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("Client connected\n");

        // Fork to handle client connection in a child process
        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process: handle client connection
            close(sockfd); // Close listening socket in child process
            handle_client(connfd, FILE_NAME);
            exit(EXIT_SUCCESS); // Terminate child process after handling client
        }
        else if (pid < 0)
        {
            perror("Fork failed");
            close(connfd); // Close connection in parent process if fork failed
        }
        else
        {
            // Parent process: continue listening for new connections
            close(connfd); // Close connection in parent process
        }
    }

    close(sockfd);
    return 0;
}
