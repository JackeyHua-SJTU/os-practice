#include "inode.h"
#include "superblock.h"
#include "wrapper.h"
#include "disc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define MAX_ARG 8
#define MAX_BUFFER_LENGTH 50000
#define ROOT_ID 0

uint16_t cur_inode_id = UINT16_MAX;
char buffer_main[MAX_BUFFER_LENGTH];
char path_name[MAX_BUFFER_LENGTH];
int initialized = 0;
static int sockfd;
int client_id = -1;

void parseCmd(char*);
void fOp();
void sig_handler(int signo) {
    if (signo == SIGINT) {
        decontructor(&d);
        // exit(EXIT_SUCCESS);
    }
}

// TODO: Multi-user may format the disc server for each other
// * Maybe can be solved only allowing the ROOT to format, namely sudo authority

int main(int argc, char** argv) {
    if (argc != 4) {
        printf("Usage: ./FS [disc server address] [BDS port] [FS port].\n");
        exit(-1);
    }
    signal(SIGINT, sig_handler);
    int disc_port = atoi(argv[2]);
    int fs_port = atoi(argv[3]);
    struct sockaddr_in disc_servaddr;
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(-1);
    }

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

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

    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    int recv_len = recv(sockfd, buffer_main, MAX_BUFFER_LENGTH - 1, 0);
    if (recv_len < 0) {
        perror("Receive failed");
        exit(-1);
    }
    char* token = strtok(buffer_main, " \n\r");
    char* name;
    int c, r;
    double dly;
    int i = 0;
    while (token != NULL) {
        // printf("%s\n", token);
        switch (i)
        {
        case 0:
            name = token;
            break;
        
        case 1:
            c = atoi(token);
            break;
        
        case 2:
            r = atoi(token);
            break;

        case 3:
            dly = strtod(token, NULL);
            break;

        case 4:
            // memset(path_name, 0, MAX_BUFFER_LENGTH);
            // strcpy(path_name, token);
            // initialized = 1;
            break;
        default:
            break;
        }
        token = strtok(NULL, " \n\r");
        ++i;
    }

    constructor(c, r, dly, name, &d);
    // fOp();

    // printf("disc %d %d %f %s %s\n", d._numCylinder, d._numSectorPerCylinder, d._trackDelay, d._file, d._discfile);

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
        // printf("Connect to client %d\n", client_id);
        // pid_t pid = fork();
        // if (pid == -1) {
        //     perror("fork");
        //     close(connfd);
        //     close(listenfd);
        //     exit(EXIT_FAILURE);
        // }

        // if (pid == 0) {
            // close(listenfd);
            char input[MAX_BUFFER_LENGTH];
            // dup2(connfd, 1);
            dup2(connfd, 2);
            char info[MAX_BUFFER_LENGTH];
            // snprintf(info, sizeof(info), "Mini-FS:\033[1;34m%s\033[0m$ ", path_name);
            // printf("%s", info);
            // fflush(stdout);
            while (1) {
                memset(info, 0, MAX_BUFFER_LENGTH);
                snprintf(info, sizeof(info), "\033[1;32mMini-FS\033[0m:\033[1;34m%s\033[0m$ ", path_name);
                send(connfd, info, strlen(info), 0);
                // fflush(stdout);
                memset(input, 0, MAX_BUFFER_LENGTH);
                int recv_len = recv(connfd, input, MAX_BUFFER_LENGTH - 1, 0);
                if (recv_len < 0) {
                    perror("Receive failed");
                    exit(-1);
                }
                printf("receive input %s\n", input);
                // fflush(stdout);
                memset(buffer_main, 0, MAX_BUFFER_LENGTH);
                parseCmd(input);
                // fflush(stdout);
                // fflush(stderr);
                if (strcmp(strtok(input, " \n\r"), "e") == 0) {
                    break;
                }
                printf("Ready to wb buffer %s\n", buffer_main);
                strcat(buffer_main, "\n");
                send(connfd, buffer_main, strlen(buffer_main), 0);
            }
            close(connfd);
        //     exit(0);
        
        // close(connfd);
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
    printf("fop called\n");
    // query(sockfd, ROOT_ID);
    // char buf[BLOCK_SIZE];
    // memset(buf, 0, BLOCK_SIZE);
    // memcpy(buf, buffer[ROOT_ID], BLOCK_SIZE);
    // int c, r;
    // if (sscanf(buf, "%d %d", &c, &r) != 2) {
    //     perror("Failed to parse the query result.");
    //     exit(-1);
    // }
    // printf("c: %d, r: %d\n", c, r);
    init_spbk(d._numCylinder * d._numSectorPerCylinder, sockfd);
    printf("Success init spbk\n");
    init_root(sockfd);
    printf("Success init root\n");
    cur_inode_id = 0;
    for (int i = 1; i < MAX_INODE_NUM; ++i) {
        // printf("Init inode %d\n", i);
        init_inode(&inode_table[i], 0, 0, 0, i, UINT16_MAX, 0, 0);
    }
    memset(path_name, 0, MAX_BUFFER_LENGTH);
    strcpy(path_name, "/");
    initialized = 1;
    printf("Success Init\n");
    char info[MAX_BUFFER_LENGTH];
    snprintf(info, sizeof(info), "%s", path_name);
    send(sockfd, info, strlen(info), 0);
}

void lsOp() {
    if (!initialized) {
        perror("File system not initialized.");
        return;
    }
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    dir_ls(&inode_table[cur_inode_id], buffer_main);
    // printf("%s\n", buffer_main);
}

void eOp() {
    printf("Good Bye\n");
    char temp[MAX_BUFFER_LENGTH];
    memset(temp, 0, MAX_BUFFER_LENGTH);
    strcat(temp, path_name);
    strcat(temp, " ");
    strcat(temp, "Q");
    send(sockfd, temp, strlen(temp), 0);
    // exit(1);
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
    int res = dir_add_entry(&inode_table[cur_inode_id], client_id, name, 0);
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    if (res == 0) {
        strcat(buffer_main, "File has been created before.");
    } else if (res == 1) {
        strcat(buffer_main, "Successfully make a new file named ");
        strcat(buffer_main, name);
    }
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
    int res = dir_add_entry(&inode_table[cur_inode_id], client_id, name, 1);
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    if (res == 0) {
        strcat(buffer_main, "Directory has been created before.");
    } else if (res == 1) {
        strcat(buffer_main, "Successfully make a new directory named ");
        strcat(buffer_main, name);
    }
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
    int res = dir_del_entry(&inode_table[cur_inode_id], client_id, name);
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    if (res == -1) {
        strcat(buffer_main, "Fail to remove: Check the type of the file.");
    } else if (res == 0) {
        strcat(buffer_main, "File has been removed before or do not exist.");
    } else {
        strcat(buffer_main, "Successfully remove the file named ");
        strcat(buffer_main, name);
    }
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
    int res = dir_del_entry_recursive(&inode_table[cur_inode_id], client_id, name);
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    if (res == -1) {
        strcat(buffer_main, "Fail to remove: Check the type of the directory.");
    } else if (res == 0) {
        strcat(buffer_main, "Directory has been removed before or do not exist.");
    } else {
        strcat(buffer_main, "Successfully remove the directory named ");
        strcat(buffer_main, name);
    }
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
    // int inode_id = dir_check_existence(&inode_table[cur_inode_id], 1, name);
    // if (inode_id == -1) {
    //     perror("Directory not found.");
    //     return;
    // }
    uint16_t backup = cur_inode_id;
    // cur_inode_id = inode_id;
    // printf("name is %s\n", name);
    char* token = strtok(name, "/\n\r");
    char backup_path[MAX_BUFFER_LENGTH];
    memset(backup_path, 0, MAX_BUFFER_LENGTH);
    strcpy(backup_path, path_name);
    while (token != NULL) {
        printf("token is %s\n", token);
        if (strcmp(token, "..") == 0) {
            uint16_t pa_id = inode_table[cur_inode_id]._fa_index;
            printf("pa_id is %d\n", pa_id);
            if (pa_id != UINT16_MAX) {
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
        token = strtok(NULL, "/\n\r");
    }
    printf("Path name is %s\n", path_name);
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    strcat(buffer_main, "Successfully change to directory ");
    strcat(buffer_main, name);
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
    // printf("%s\n", buffer_main);
}

void writeOpe(char* file_name, char* data) {
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
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    strcat(buffer_main, "Successfully write");
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
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    strcat(buffer_main, "Successfully insert");
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
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    strcat(buffer_main, "Successfully delete");
}

void parseCmd(char* command) {
    char* token = strtok(command, " \n\r");
    char** argv = malloc(MAX_ARG * sizeof(char*));
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
            // free(argv);
            // exit(-1);
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
            // free(argv);
            // exit(-1);
        }
    } else if (i == 4) {
        if (strcmp(argv[0], "w") == 0) {
            int len = atoi(argv[2]);
            char data[len + 1];
            memset(data, 0, len + 1);
            strncpy(data, argv[3], len);
            data[len] = '\0';
            writeOpe(argv[1], data);
        } else if (strcmp(argv[0], "d") == 0) {
            int pos = atoi(argv[2]);
            int len = atoi(argv[3]);
            deleteOp(argv[1], pos, len);
        } else {
            perror("Invalid command.");
            // free(argv);
            // exit(-1);
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
            // free(argv);
            // exit(-1);
        }
    } else {
        perror("Invalid command.");
        // free(argv);
        // exit(-1);
    }
    free(argv);
}