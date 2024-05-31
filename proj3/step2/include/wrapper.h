/*
    * Wrapper Function Declaration
*/
// * Wrap disc read/write/query operations
// * Supply a much more user-friendly interface for file system
#ifndef _WRAPPER_H_
#define _WRAPPER_H_

#define MAX_CLIENTS 6
#define MAX_LENGTH 1024

#include <stdint.h>
#include "disc.h"   

// * Write the response of disc server to the correspond index
// ! Notice the cliend 0 is the root user
// ! Those id >= 1 are real clients
extern char buffer[MAX_CLIENTS][MAX_LENGTH];
extern char cmd[MAX_LENGTH];
extern Disc d;

void query(int sockfd, uint16_t client_id);

void read2(int sockfd, uint16_t client_id, int c, int r);

void read1(int sockfd, uint16_t client_id, int index);

// * We need to handle arbitrary length
void write2(int sockfd, uint16_t client_id, int c, int r, long size, char* data);

void write1(int sockfd, uint16_t client_id, int index, long size, void* data);

#endif