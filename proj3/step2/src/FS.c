#include "inode.h"
#include "superblock.h"
#include "wrapper.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_ARGS 8
#define MAX_BUFFER_LENGTH 30000
#define ROOT_ID 0

uint16_t cur_inode_id = UINT16_MAX;
char buffer_main[MAX_BUFFER_LENGTH];
char path_name[MAX_BUFFER_LENGTH];
int initialized = 0;
static int sockfd;
int client_id = -1;

void parseCmdInput(char*);

// TODO: Multi-user may format the disc server for each other
// * Maybe can be solved only allowing the ROOT to format, namely sudo authority

int main(int argc, char** argv) {
    if (argc != 4) {
        printf("Usage: ./FS [disc server address] [BDS port] [FS port].\n");
        exit(-1);
    }
    int disc_port = atoi(argv[2]);
    int fs_port = atoi(argv[3]);
    struct sockaddr_in disc_servaddr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(-1);
    }

    memset(&disc_servaddr, 0, sizeof(disc_servaddr));
    disc_servaddr.sin_family = AF_INET;
    disc_servaddr.sin_port = htons(disc_port);

    if (inet_pton(AF_INET, argv[1], &disc_servaddr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(-1);
    }

    if (connect(sockfd, (struct sockaddr*)&disc_servaddr, sizeof(disc_servaddr)) < 0) {
        perror("Connection Failed");
        exit(-1);
    }

    printf("Connected to the disc server.\n");

    // * Then make this file system a server.
    int listenfd, connfd;
    int optval = 1;
    struct sockaddr_in servaddr, client_addr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        perror("setsockopt");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(fs_port);

    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, MAX_CLIENTS) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        socklen_t client_len = sizeof(client_addr);
        connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_len);

        if (connfd == -1) {
            perror("accept");
            continue;
        }
        ++client_id;
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            close(connfd);
            close(listenfd);
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            close(listenfd);
            char input[MAX_BUFFER_LENGTH];
            dup2(connfd, 1);
            dup2(connfd, 2);
            while (1) {
                memset(input, 0, MAX_BUFFER_LENGTH);
                int recv_len = recv(connfd, input, MAX_BUFFER_LENGTH - 1, 0);
                if (recv_len < 0) {
                    perror("Receive failed");
                    exit(-1);
                }
                parseCmdInput(input);
                fflush(stdout);
                fflush(stderr);
            }
            close(connfd);
            exit(0);
        }
        close(connfd);
    }
}

int validate_name(char* name) {
    for (int i = 0; i < strlen(name); ++i) {
        // * We only allow normal name, and `.` for cur dir and `..` for parent dir.
        if ((name[i] >= 'a' && name[i] <= 'z') || (name[i] >= 'A' && name[i] <= 'Z') || (name[i] >= '0' && name[i] <= '9') || name[i] == '_' || name[i] == '.') {
            continue;
        } else {
            return 0;
        }
    }
    return 1;
}

void fOp() {
    query(sockfd, ROOT_ID);
    char buf[BLOCK_SIZE];
    memset(buf, 0, BLOCK_SIZE);
    memcpy(buf, buffer[ROOT_ID], BLOCK_SIZE);
    int c, r;
    if (sscanf(buf, "%d %d", &c, &r) != 2) {
        perror("Failed to parse the query result.");
        exit(-1);
    }
    init_spbk(c * r, sockfd);
    init_root(sockfd);
    cur_inode_id = 0;
    for (int i = 1; i < MAX_INODE_NUM; ++i) {
        init_inode(&inode_table[i], 0, 0, 0, i, UINT16_MAX, 0, 0);
    }
    memset(path_name, 0, MAX_BUFFER_LENGTH);
    strcpy(path_name, "/");
    initialized = 1;
    printf("Success Init\n");
}

void lsOp() {
    if (!initialized) {
        perror("File system not initialized.");
        return;
    }
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    dir_ls(&inode_table[cur_inode_id], buffer_main);
    printf("%s\n", buffer_main);
}

void eOp() {
    printf("Good Bye\n");
    exit(1);
}

void mkOp(char* name) {
    if (!initialized) {
        perror("File system not initialized.");
        return;
    }
    if (!validate_name(name)) {
        perror("Invalid name.");
        return;
    }
    // * In `dir_add_entry`, we have checked whether a file with same name exists.
    dir_add_entry(&inode_table[cur_inode_id], client_id, name, 0);
}

void mkdirOp(char* name) {
    if (!initialized) {
        perror("File system not initialized.");
        return;
    }
    if (!validate_name(name)) {
        perror("Invalid name.");
        return;
    }
    // * In `dir_add_entry`, we have checked whether a file with same name exists.
    dir_add_entry(&inode_table[cur_inode_id], client_id, name, 1);
}

void rmOp(char* name) {
    if (!initialized) {
        perror("File system not initialized.");
        return;
    }
    if (!validate_name(name)) {
        perror("Invalid name.");
        return;
    }
    dir_del_entry(&inode_table[cur_inode_id], client_id, name);
}

void rmdirOp(char* name) {
    if (!initialized) {
        perror("File system not initialized.");
        return;
    }
    if (!validate_name(name)) {
        perror("Invalid name.");
        return;
    }
    dir_del_entry_recursive(&inode_table[cur_inode_id], client_id, name);
}

void cdOp(char* name) {
    if (!initialized) {
        perror("File system not initialized.");
        return;
    }
    // if (!validate_name(name)) {
    //     perror("Invalid name.");
    //     return;
    // }
    int inode_id = dir_check_existence(&inode_table[cur_inode_id], 1, name);
    if (inode_id == -1) {
        perror("Directory not found.");
        return;
    }
    uint16_t backup = cur_inode_id;
    cur_inode_id = inode_id;
    char* token = strtok(path_name, "/");
    char backup_path[MAX_BUFFER_LENGTH];
    memset(backup_path, 0, MAX_BUFFER_LENGTH);
    strcpy(backup_path, path_name);
    while (token != NULL) {
        if (strcmp(token, "..") == 0) {
            uint16_t pa_id = inode_table[cur_inode_id]._fa_index;
            if (pa_id != UINT16_MAX && pa_id != inode_table[pa_id]._fa_index) {
                // * Do have a parent dir
                cur_inode_id = pa_id;
                char* last_occur = strrchr(path_name, '/');
                if (last_occur != NULL) {
                    *last_occur = '\0';
                }
                if (pa_id == ROOT_ID) {
                    memset(path_name, 0, MAX_BUFFER_LENGTH);
                    strcpy(path_name, "/");
                }
            }
        } else if (strcmp(token, ".") != 0) {
            int child_id = dir_check_existence(&inode_table[cur_inode_id], 1, token);
            if (child_id == -1) {
                perror("Directory not found.");
                cur_inode_id = backup;
                strcpy(path_name, backup_path);
                return;
            } else {
                if (cur_inode_id != ROOT_ID) {
                    strcat(path_name, "/");
                }
                cur_inode_id = child_id;
                strcat(path_name, token);
            }
        }
        token = strtok(NULL, "/");
    }
}

void catOp(char* name) {
    if (!initialized) {
        perror("File system not initialized.");
        return;
    }
    if (!validate_name(name)) {
        perror("Invalid name.");
        return;
    }
    int file_inode_id = dir_check_existence(&inode_table[cur_inode_id], 0, name);
    if (file_inode_id == -1) {
        perror("File not found.");
        return;
    }
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    file_read(&inode_table[file_inode_id], client_id, buffer_main);
    printf("%s\n", buffer_main);
}

void writeOp(char* file_name, char* data) {
    if (!initialized) {
        perror("File system not initialized.");
        return;
    }
    if (!validate_name(file_name)) {
        perror("Invalid name.");
        return;
    }
    int file_inode_id = dir_check_existence(&inode_table[cur_inode_id], 0, file_name);
    if (file_inode_id == -1) {
        perror("File not found.");
        return;
    }
    file_write(&inode_table[file_inode_id], client_id, data);
}

void insertOp(char* name, int pos, char* data) {
    if (!initialized) {
        perror("File system not initialized.");
        return;
    }
    if (!validate_name(name)) {
        perror("Invalid name.");
        return;
    }
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    int file_inode_id = dir_check_existence(&inode_table[cur_inode_id], 0, name);
    if (file_inode_id == -1) {
        perror("File not found.");
        return;
    }
    file_read(&inode_table[file_inode_id], client_id, buffer_main);
    if (pos < 0) {
        perror("Invalid position.");
        return;
    }
    if (pos >= strlen(buffer_main)) {
        strcat(buffer_main, data);
    } else {
        memmove(buffer_main + pos + strlen(data), buffer_main + pos, strlen(buffer_main) - pos + 1);
        strncpy(buffer_main + pos, data, strlen(data));
    }
    file_write(&inode_table[file_inode_id], client_id, buffer_main);
}

void deleteOp(char* name, int pos, int len) {
    if (!initialized) {
        perror("File system not initialized.");
        return;
    }
    if (!validate_name(name)) {
        perror("Invalid name.");
        return;
    }
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    int file_inode_id = dir_check_existence(&inode_table[cur_inode_id], 0, name);
    if (file_inode_id == -1) {
        perror("File not found.");
        return;
    }
    file_read(&inode_table[file_inode_id], client_id, buffer_main);
    if (pos < 0 || pos >= strlen(buffer_main)) {
        perror("Invalid position.");
        return;
    }
    if (len <= 0) return;
    if (pos + len >= strlen(buffer_main)) {
        buffer_main[pos] = '\0';
    } else {
        memmove(buffer_main + pos, buffer_main + pos + len, strlen(buffer_main) - pos - len + 1);
    }
    file_write(&inode_table[file_inode_id], client_id, buffer_main);
}

void parseCmdInput(char* command) {
    char* token = strtok(command, " \n\r");
    char** argv = malloc(MAX_ARGS * sizeof(char*));
    int i = 0;
    while (token != NULL) {
        argv[i++] = token;
        token = strtok(NULL, " \n\r");
    }
    argv[i] = NULL;
    if (i == 1) {
        if (strcmp(argv[0], "f") == 0) {
            fOp();
        } else if (strcmp(argv[0], "ls") == 0) {
            lsOp();
        } else if (strcmp(argv[0], "e") == 0) {
            eOp();
        } else {
            perror("Invalid command.");
            free(argv);
            exit(-1);
        }
    } else if (i == 2) {
        if (strcmp(argv[0], "mk") == 0) {
            mkOp(argv[1]);
        } else if (strcmp(argv[0], "mkdir") == 0) {
            mkdirOp(argv[1]);
        } else if (strcmp(argv[0], "rm") == 0) {
            rmOp(argv[1]);
        } else if (strcmp(argv[0], "rmdir") == 0) {
            rmdirOp(argv[1]);
        } else if (strcmp(argv[0], "cd") == 0) {
            cdOp(argv[1]);
        } else if (strcmp(argv[0], "cat") == 0) {
            catOp(argv[1]);
        } else {
            perror("Invalid command.");
            free(argv);
            exit(-1);
        }
    } else if (i == 4) {
        if (strcmp(argv[0], "w") == 0) {
            int len = atoi(argv[2]);
            char data[len + 1];
            memset(data, 0, len + 1);
            strncpy(data, argv[3], len);
            data[len] = '\0';
            writeOp(argv[1], data);
        } else if (strcmp(argv[0], "d") == 0) {
            int pos = atoi(argv[2]);
            int len = atoi(argv[3]);
            deleteOp(argv[1], pos, len);
        } else {
            perror("Invalid command.");
            free(argv);
            exit(-1);
        }
    } else if (i == 5) {
        if (strcmp(argv[0], "i") == 0) {
            int pos = atoi(argv[2]);
            int len = atoi(argv[3]);
            char data[len + 1];
            memset(data, 0, len + 1);
            strncpy(data, argv[4], len);
            data[len] = '\0';
            insertOp(argv[1], pos, data);
        } else {
            perror("Invalid command.");
            free(argv);
            exit(-1);
        }
    } else {
        perror("Invalid command.");
        free(argv);
        exit(-1);
    }
}