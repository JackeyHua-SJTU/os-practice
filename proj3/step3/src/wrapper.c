#include "disc.h"
#include "user.h"
#include "wrapper.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

Disc d;
char buffer[MAX_CLIENTS][MAX_LENGTH];
char cmd[MAX_LENGTH];
User user[8];
int user_count;

// ! Socket connection should be done at main function
// TODO: Check whether \n received from the server matter

#define BLOCK_SIZE 256

void query(int sockfd, uint16_t client_id) {
    memset(buffer[client_id], 0, sizeof(buffer[client_id]));
    memset(cmd, 0, sizeof(cmd));
    send(sockfd, "I", 1, 0);
    recv(sockfd, buffer[client_id], MAX_LENGTH - 1, 0);
}

void read2(int sockfd, uint16_t client_id, int c, int r) {
    memset(buffer[client_id], 0, sizeof(buffer[client_id]));
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "R %d %d\n", c, r);
    send(sockfd, cmd, sizeof(cmd), 0);
    recv(sockfd, buffer[client_id], MAX_LENGTH - 1, 0);
    char* space = strchr(buffer[client_id], ' ');
    if (space != NULL) {
        memmove(buffer[client_id], space + 1, strlen(space + 1) + 1);
    }
}

void read1(int sockfd, uint16_t client_id, int index) {
    // memset(buffer[client_id], 0, sizeof(buffer[client_id]));
    // memset(cmd, 0, sizeof(cmd));
    // sprintf(cmd, "R %d\n", index);
    // send(sockfd, cmd, sizeof(cmd), 0);
    // recv(sockfd, buffer[client_id], MAX_LENGTH - 1, 0);
    // char* space = strchr(buffer[client_id], ' ');
    // if (space != NULL) {
    //     memmove(buffer[client_id], space + 1, strlen(space + 1) + 1);
    // }
    memset(buffer[client_id], 0, sizeof(buffer[client_id]));
    char* arr = readOp_client(&d, index);
    memcpy(buffer[client_id], arr, BLOCK_SIZE);
    free(arr);
}

void write2(int sockfd, uint16_t client_id, int c, int r, long size, char* data) {
    for (int i = 0; i < (size + BLOCK_SIZE - 1) / BLOCK_SIZE; ++i) {
        memset(buffer[client_id], 0, sizeof(buffer[client_id]));
        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "W %d %d %d %s\n", c, r + i, BLOCK_SIZE, data + i * BLOCK_SIZE);
        send(sockfd, cmd, sizeof(cmd), 0);
        recv(sockfd, buffer[client_id], MAX_LENGTH - 1, 0);
    }
}

void write1(int sockfd, uint16_t client_id, int index, long size, void* data) {
    int rnd = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    for (int i = 0; i < rnd; ++i) {
        // printf("i = %ld\n", i);
        // memset(buffer[client_id], 0, sizeof(buffer[client_id]));
        // memset(cmd, 0, sizeof(cmd));
        // printf("data is %s\n", data + i * BLOCK_SIZE);
        // // printf("len of cur is %ld\n", strlen(data + i * BLOCK_SIZE));
        // int cmd_len = snprintf(cmd, sizeof(cmd), "W %d %d\n", index + i, BLOCK_SIZE);
        // printf("cmd len is %d\n", cmd_len);
        // // memcpy(cmd + cmd_len, data + i * BLOCK_SIZE, BLOCK_SIZE);
        // // for (int j = 0; j < cmd_len + BLOCK_SIZE; ++j) {
        // //     printf("%02x", (unsigned char)cmd[j]);
        // // }
        // int ans = send(sockfd, cmd, cmd_len, 0);
        // send(sockfd, data + i * BLOCK_SIZE, BLOCK_SIZE, 0);
        // printf("finish send\n");
        // if (ans < 0) {
        //     if (errno == EWOULDBLOCK) {
        //         printf("Send timeout\n");
        //     } else {
        //         printf("Send failed");
        //     }
        // }
        // printf("Send finish\n");
        // recv(sockfd, buffer[client_id], MAX_LENGTH - 1, 0);
        writeOp_client(&d, index + i, BLOCK_SIZE, data + i * BLOCK_SIZE);
    }
    fflush(stdout);
}