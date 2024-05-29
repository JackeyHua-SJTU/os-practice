#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define RESPONSE_SIZE 30000
#define MAX_CMD_SIZE 512

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: [server IP] [port].\n");
        exit(-1);
    }

    int sockfd;
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(-1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2])); 

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(-1);
    }

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Connection Failed");
        exit(-1);
    }

    printf("Connected to the server.\n");
    char command[MAX_CMD_SIZE];
    char response[RESPONSE_SIZE];
    char wd[RESPONSE_SIZE];

    while (1) {
        memset(wd, 0, RESPONSE_SIZE);
        int bytes_received = recv(sockfd, wd, RESPONSE_SIZE - 1, 0);
        printf("%s", wd);
        // fflush(stdout);
        memset(command, 0, MAX_CMD_SIZE);
        fgets(command, MAX_CMD_SIZE, stdin);
        // printf("Receive command %s\n", command);
        if (send(sockfd, command, strlen(command), 0) < 0) {
            perror("Send failed");
            exit(-1);
        }
        memset(response, 0, RESPONSE_SIZE); 

        // int recv_len = recv(sockfd, response, RESPONSE_SIZE - 1, 0); 
        // int recv_len = recv(sockfd, response, RESPONSE_SIZE - 1, 0);
        // if (recv_len < 0) {
        //     perror("Receive failed");
        //     exit(-1);
        // } else if (recv_len == 0) {
        //     break;
        // }
        if (strcmp(strtok(command, " \n\r"), "e") == 0) {
            break;
        }
        
        int totalRead = 0;
        int read21;
        while (1) {
            read21 = read(sockfd, response + totalRead, 1);
            if (read21 < 0) {
                perror("Receive failed");
                exit(-1);
            } else if (read21 == 0) {
                break;
            }
            totalRead += read21;
            // printf("totalRead = %d\n", totalRead);
            if (response[totalRead - 1] == '\n') break;
            // response[totalRead] = '\0';
        }

        printf("%s\n", response);
        if (strcmp(strtok(command, " \n\r"), "e") == 0) {
            break;
        }
        // fflush(stdout);
        // fflush(stderr);
        
    }

    close(sockfd);
    return 0;
}