/*
    * Basic Disc Client 
    * Random input
*/
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#define MAX_INPUT 512
#define MAX_RESPONSE 1024

int cyclinderNum;
int sectorPerCylinder;

void parseArg(char* cmd);
void genRanData(char* data);

int main(int argc, char** argv) {
    if (argc != 4) {
        printf("Usage, [server IP] [port] [num of requests].\n");
        exit(-1);
    }
    srand(time(NULL));
    int N = atoi(argv[3]);

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

    char cmd[MAX_INPUT];
    char response[MAX_RESPONSE];

    send(sockfd, "I", 1, 0);
    memset(response, 0, sizeof(response));
    recv(sockfd, response, sizeof(response) - 1, 0);
    parseArg(response);

    // TODO: Add a decent end way when server sends error info and exit(-1);
    for (int i = 0; i < N; ++i) {
        int op = rand() % 3;
        memset(cmd, 0, MAX_INPUT);
        memset(response, 0, MAX_RESPONSE);
        switch (op)
        {
        case 0:
            sprintf(cmd, "I\n");
            send(sockfd, cmd, sizeof(cmd), 0);
            recv(sockfd, response, sizeof(response) - 1, 0);
            printf("Operation %d is I, Success\n", i);
            break;
        
        case 1:
            int c = rand() % cyclinderNum;
            int r = rand() % sectorPerCylinder;
            sprintf(cmd, "R %d %d\n", c, r);
            send(sockfd, cmd, sizeof(cmd), 0);
            recv(sockfd, response, sizeof(response) - 1, 0);
            // printf("Response in read is %s\n", response);
            printf("Operation %d is R %d %d, Success\n", i, c, r);
            break;

        case 2:
            c = rand() % cyclinderNum;
            r = rand() % sectorPerCylinder;
            char data[257];
            memset(data, 0, sizeof(data));
            genRanData(data);
            // printf("Data in case 2: %s\n", data);
            sprintf(cmd,"W %d %d %ld %s\n", c, r, strlen(data), data);
            // printf("Cmd is %s\n", cmd);
            send(sockfd, cmd, sizeof(cmd), 0);
            recv(sockfd, response, sizeof(response) - 1, 0);
            // printf("Response in write is %s\n", response);
            printf("Operation %d is W %d %d, Success\n", i, c, r);

        default:
            break;
        }
    }
}

void parseArg(char* cmd) {
    char* token = strtok(cmd, " \n\r");
    cyclinderNum = atoi(token);
    token = strtok(NULL, " ");
    sectorPerCylinder = atoi(token);
}

void genRanData(char* data) {
    for (int i = 0; i < 256; ++i) {
        data[i] = 'a' + rand() % 26;
    }
    // printf("Data: %s\n", data);
    // * Add \0 to mark the end of the string
    data[256] = '\0';
}