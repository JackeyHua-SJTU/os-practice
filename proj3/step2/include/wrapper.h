// * Wrap disc read/write/query operations
// * Supply a much more user-friendly interface for file system
#ifndef _WRAPPER_H_
#define _WRAPPER_H_

#define MAX_CLENTS 6
#define MAX_LENGTH 1024

// * Write the response of disc server to the correspond index
char buffer[MAX_CLENTS][MAX_LENGTH];
char cmd[MAX_LENGTH];

void query(int sockfd, int client_id);

void read(int sockfd, int client_id, int c, int r);

// * We need to handle arbitrary length
void write(int sockfd, int client_id, int c, int r, long size, char* data);

#endif