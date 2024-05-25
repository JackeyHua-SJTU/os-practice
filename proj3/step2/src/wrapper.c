#include "wrapper.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>

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
    memset(buffer[client_id], 0, sizeof(buffer[client_id]));
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "R %d\n", index);
    send(sockfd, cmd, sizeof(cmd), 0);
    recv(sockfd, buffer[client_id], MAX_LENGTH - 1, 0);
    char* space = strchr(buffer[client_id], ' ');
    if (space != NULL) {
        memmove(buffer[client_id], space + 1, strlen(space + 1) + 1);
    }
}

void write2(int sockfd, uint16_t client_id, int c, int r, long size, char* data) {
    for (int i = 0; i < size / BLOCK_SIZE; ++i) {
        memset(buffer[client_id], 0, sizeof(buffer[client_id]));
        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "W %d %d %d %s\n", c, r + i, BLOCK_SIZE, data + i * BLOCK_SIZE);
        send(sockfd, cmd, sizeof(cmd), 0);
        recv(sockfd, buffer[client_id], MAX_LENGTH - 1, 0);
    }
}

void write1(int sockfd, uint16_t client_id, int index, long size, char* data) {
    for (int i = 0; i < size / BLOCK_SIZE; ++i) {
        memset(buffer[client_id], 0, sizeof(buffer[client_id]));
        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "W %d %d %s\n", index, BLOCK_SIZE, data + i * BLOCK_SIZE);
        send(sockfd, cmd, sizeof(cmd), 0);
        recv(sockfd, buffer[client_id], MAX_LENGTH - 1, 0);
    }
}