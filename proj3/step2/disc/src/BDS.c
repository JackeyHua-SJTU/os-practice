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
#include <libgen.h>

#define MAX_INPUT 1024
#define MAX_CLIENTS 5
#define MAX_DATA_LENGTH 1024

Disc d;
int initial;
int done;
char backupPath[MAX_DATA_LENGTH];

void sig_handler(int signo) {
    if (signo == SIGINT) {
        decontructor(&d);
        // exit(EXIT_SUCCESS);
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
    initial = 0;
    done = 0;
    memset(backupPath, 0, sizeof(backupPath));
    char path[128];
    getcwd(path, sizeof(path));
    printf("path is %s\n", path);
    strcat(path, "/");
    strcat(path, argv[1]);
    printf("path is %s\n", path);
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
    printf("%s\n", argv[4]);
    // printf("Disc server is running...\n");
    // printf("Cylinder: %d\n", d._numCylinder);
    //     printf("Sector per Cylinder: %d\n", d._numSectorPerCylinder);
    //     printf("Track to Track Delay: %f\n", d._trackDelay);
    //     printf("FD: %d\n", d._fd);
    //     printf("name: %s\n", d._discfile);
    //     printf("size: %s\n", d._file);
    // * Because of multiple clients, we deal with track to track delay inside disc deal function.
    while (1) {
        socklen_t client_len = sizeof(client_addr);
        clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);

        if (clientfd == -1) {
            perror("accept");
            continue;
        }
        // pid_t pid = fork();
        // if (pid == -1) {
        //     perror("fork");
        //     close(clientfd);
        //     close(sockfd);
        //     exit(EXIT_FAILURE);
        // }

        // if (pid == 0) {
            // close(sockfd);
            char input[MAX_INPUT];
            // dup2(clientfd, 1);
            dup2(clientfd, 2);
            char info[1024];
            
                memset(info, 0, sizeof(info));
                strcat(info, path);
                strcat(info, " ");
                strcat(info, argv[2]);
                strcat(info, " ");
                strcat(info, argv[3]);
                strcat(info, " ");
                strcat(info, argv[4]);
                if (initial) {
                    strcat(info, " ");
                    strcat(info, "1");
                } else {
                    strcat(info, " ");
                    strcat(info, "0");
                }
                info[strlen(info)] = '\0';

                // snprintf(info, sizeof(info), "%s %s %s %s", path, argv[2], argv[3], argv[4]);
                // info[strlen(info) - 1] = '\0';
            // while (1) {
                // char info[32];
                // memset(info, 0, sizeof(info));
                // snprintf(info, sizeof(info), "Welcome to the disc server: ");
                // write(clientfd, info, strlen(info));
                memset(input, 0, sizeof(input));
                // void* data;
                // int n = read(clientfd, input, sizeof(input));
                // perror(input);
                // if (strcmp(strtok(input, "\n\r"), "Q") == 0) {
                //     write(clientfd, "Goodbye.\n", 9);
                //     break;
                // } else if (strcmp(strtok(input, "\n\r"), "W") == 0) {
                //     perror("write");
                //     read(clientfd, data, MAX_DATA_LENGTH);
                // }
                // perror(input);
                // // input[n] = '\0';
                // parseCmdInput(&d, input, data);
                // // * Manually flush stdout buffer
                // fflush(stdout);
                
                memcpy(input, &d, sizeof(Disc));
                write(clientfd, info, strlen(info));
                void* data;
                if (!done) { 
                    decontructor(&d);
                    done = 1;
                }
                printf("Connect to a fs client\n");
                while (1) {
                    initial = 1;
                    memset(input, 0, sizeof(input));
                    int n = read(clientfd, input, sizeof(input));
                    // printf("input is %s\n", input);
                    if (n == 0) {
                        break;
                    }
                    char* token = strtok(input, " \n\r");
                    int breakFlag = 0;
                    while (token != NULL) {
                        // printf("token is %s\n", token);
                        if (strcmp(token, "Q") == 0) {
                            breakFlag = 1;
                            break;
                        } else {
                            initial = 1;
                            memset(backupPath, 0, sizeof(backupPath));
                            strcpy(backupPath, token);
                        }
                        token = strtok(NULL, " \n\r");
                    }
                    if (breakFlag) {
                        break;
                    }
                    // if (strcmp(strtok(input, "\n\r"), "Q") == 0) {
                    //     // write(clientfd, "Goodbye.\n", 9);
                    //     break;
                    // }
                    // initial = 1;
                    // strcpy(backupPath, input);
                    // printf("Input is %s", input);
                    // input[n] = '\0';
                    // parseCmdInput(&d, input, data);
                    // * Manually flush stdout buffer
                    fflush(stdout);
                }
            // printf("backupPath is %s\n", backupPath);
            close(clientfd);
            // exit(EXIT_SUCCESS);
        // } else {
        //     close(clientfd);
        // }
    }

}