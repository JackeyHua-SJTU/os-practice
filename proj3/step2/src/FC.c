#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

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
    char command[512];
    char response[1024];

    while (1) {
        printf("Disc Query: ");
        memset(command, 0, 512);
        fgets(command, 512, stdin);
        if (send(sockfd, command, sizeof(command), 0) < 0) {
            perror("Send failed");
            exit(-1);
        }
        memset(response, 0, 1024); 
        int recv_len = recv(sockfd, response, 1023, 0); 
        if (recv_len < 0) {
            perror("Receive failed");
            exit(-1);
        } else if (recv_len == 0) {
            break;
        }
        printf("Response: %s\n", response);
        fflush(stdout);
        fflush(stderr);
        if (strcmp(strtok(command, " \n\r"), "e") == 0) {
            break;
        }
    }

}