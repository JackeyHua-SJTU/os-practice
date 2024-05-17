/*
    * Basic Disc Server
*/
#include "disc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_INPUT 512
#define MAX_CLIENTS 5

Disc d;

void sig_handler(int signo) {
    if (signo == SIGINT) {
        decontructor(&d);
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char** argv) {
    if (argc != 6) {
        printf("Usage: [filename] [# of cylinders] [# of sectors per cyclinder] [track to track delay] [port].\n");
        exit(-1);
    }
    signal(SIGINT, sig_handler);
    int port = atoi(argv[5]);
    // * Make it a server.
    int optval = 1;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int clientfd;
    if (sockfd == -1) {
        perror("socket");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, MAX_CLIENTS) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }


    constructor(atoi(argv[2]), atoi(argv[3]), strtod(argv[4], NULL), argv[1], &d);
    
    // * Because of multiple clients, we deal with track to track delay inside disc deal function.
    while (1) {
        socklen_t client_len = sizeof(client_addr);
        clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);

        if (clientfd == -1) {
            perror("accept");
            continue;
        }
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            close(clientfd);
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            close(sockfd);
            char input[MAX_INPUT];
            dup2(clientfd, 1);
            dup2(clientfd, 2);
            while (1) {
                // char info[32];
                // memset(info, 0, sizeof(info));
                // snprintf(info, sizeof(info), "Welcome to the disc server: ");
                // write(clientfd, info, strlen(info));
                memset(input, 0, sizeof(input));
                int n = read(clientfd, input, sizeof(input));
                if (strcmp(strtok(input, "\n\r"), "Q") == 0) {
                    write(clientfd, "Goodbye.\n", 9);
                    break;
                }
                // printf("Input is %s", input);
                // input[n] = '\0';
                parseCmdInput(&d, input);
                // * Manually flush stdout buffer
                fflush(stdout);
            }
            close(clientfd);
            exit(EXIT_SUCCESS);
        } else {
            close(clientfd);
        }
    }

}