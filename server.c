#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define FILE_NAME "attendance.tsv" // Changed file extension to .tsv

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    int sockfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[BUFFER_SIZE];

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

    // Open the file for writing (overwrite if exists)
    FILE *file = fopen(FILE_NAME, "w");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Write column headers to file
    fprintf(file, "Roll Number\tIP Address\tTimestamp\n");

    fclose(file); // Close the file before entering the loop

    while (1)
    {
        // Accept connection
        socklen_t len = sizeof(cliaddr);
        connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
        if (connfd < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("Client connected\n");

        // Receive roll number from client
        recv(connfd, buffer, BUFFER_SIZE, 0);

        // Get client IP address
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(cliaddr.sin_addr), client_ip, INET_ADDRSTRLEN);

        // Get current timestamp
        time_t rawtime;
        struct tm *timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

        // Open the file in append mode
        file = fopen(FILE_NAME, "a");
        if (file == NULL)
        {
            perror("Error opening file");
            close(connfd);
            continue;
        }

        // Write roll number, IP address, and timestamp to file
        fprintf(file, "%s\t%s\t%s\n", buffer, client_ip, timestamp);

        fclose(file); // Close the file after writing

        send(connfd, "Attendance Marked", 17, 0);

        printf("Attendance recorded for roll number %s\n", buffer);

        close(connfd);
    }

    close(sockfd);
    return 0;
}
