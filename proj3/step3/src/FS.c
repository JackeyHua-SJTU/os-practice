/*
    * File system server
*/
#include "inode.h"
#include "superblock.h"
#include "wrapper.h"
#include "disc.h"
#include "user.h"
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
#include <stdint.h>

#define MAX_ARG 8
#define MAX_BUFFER_LENGTH 50000
#define ROOT_ID 0

// * We need buffer_main to be a two-dim array, the first dimention is the number of clients MAX_CLIENTS
// * Same for the path_name and cur_inode_id

// TODO: support login and cr function and chmod 
// TODO: all user home dir will be /home/{USER_NAME}
// TODO: 

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
        exit(EXIT_SUCCESS);
    }
}

// * We need to create a process that operates on the disc
// * Child process interacts with the disc via pipe

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
                initialized = atoi(token);
                printf("init from disc is %d\n", initialized);
                break;
            default:
                break;
            }
            token = strtok(NULL, " \n\r");
            ++i;
        }

        constructor(c, r, dly, name, &d);
        printf("success constructor, with c : %d, r : %d\n", c, r);
        // fOp();

        if (initialized == 1) {
                cur_inode_id = 0;
                memset(path_name, 0, MAX_BUFFER_LENGTH);
                strcpy(path_name, "/");
                memset(&spbk, 0, sizeof(Superblock));
                memset(inode_table, 0, sizeof(Inode) * MAX_INODE_NUM);
                char bufff[256 * 3];
                for (int i = 0; i < 3; ++i) {
                    char* temp = readOp_client(&d, i);
                    memcpy(bufff + i * 256, temp, 256);
                }
                memcpy(&spbk, bufff, sizeof(Superblock));
                printf("inode_count: %d\n", spbk._inode_count);
                printf("block_count: %d\n", spbk._block_count);
                printf("vacant_inode_count: %d\n", spbk._vacant_inode_count);
                printf("vacant_block_count: %d\n", spbk._vacant_block_count);
                char buffff[256];
                memset(buffff, 0, 256);
                for (int i = 0; i < 128; ++i) {
                    char* temp = readOp_client(&d, i + 3);
                    memcpy(buffff, temp, 256);
                    memcpy(&inode_table[i * 8], buffff, 256);
                }
                // memset(buffff, 0, 256);
                // char* temp = readOp_client(&d, 131);
                // memcpy(buffff, temp, 256);
                // memcpy(user, buffff, 256);
                // printf("user[1] has name %s\n", user[1]._user_id);
                user_count = 6;
        } else {
            fOp();
        }

    // int pipefd[2];
    // int wbfd[2];
    // if (pipe(pipefd) == -1) {
    //     perror("pipe");
    //     exit(EXIT_FAILURE);
    // }
    // if (pipe(wbfd) == -1) {
    //     perror("pipe");
    //     exit(EXIT_FAILURE);
    // }
    // pid_t pid = fork();
    // if (pid == -1) {
    //     perror("fork");
    //     close(pipefd[0]);
    //     close(pipefd[1]);
    //     exit(EXIT_FAILURE);
    // }

    // if (pid == 0) {
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
            // ++client_id;
            // printf("Connect to client %d, whose connfd is %d\n", client_id, connfd);
            // pid_t pid_2 = fork();
            // if (pid_2 == -1) {
            //     perror("fork");
            //     close(connfd);
            //     close(listenfd);
            //     exit(EXIT_FAILURE);
            // }

            // if (pid_2 == 0) {
            //     close(pipefd[0]);
            //     close(wbfd[1]);
            //     close(listenfd);
                char input[MAX_BUFFER_LENGTH];
                // dup2(connfd, 1);
                dup2(connfd, 2);
                char info[MAX_BUFFER_LENGTH];
                cur_inode_id = 0;
                client_id = 0;
                memset(path_name, 0, MAX_BUFFER_LENGTH);
                strcpy(path_name, "/");
                // snprintf(info, sizeof(info), "Mini-FS:\033[1;34m%s\033[0m$ ", path_name);
                // printf("%s", info);
                // fflush(stdout);
                while (1) {
                    memset(info, 0, MAX_BUFFER_LENGTH);
                    snprintf(info, sizeof(info), "\033[1;32mMini-FS\033[0m:\033[1;34m%s\033[0m$ ", path_name);
                    send(connfd, info, strlen(info), 0);

                    memset(input, 0, MAX_BUFFER_LENGTH);
                    int recv_len = recv(connfd, input, MAX_BUFFER_LENGTH - 1, 0);
                    if (recv_len < 0) {
                        perror("Receive failed");
                        exit(-1);
                    }
                    printf("receive input %s from client %d\n", input, client_id);
                    // fflush(stdout);
                    memset(info, 0, MAX_BUFFER_LENGTH);
                    sprintf(info, "%d %s", client_id, input);
                    printf("receive input %s\n", info);
                    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
                    parseCmd(info);
                    // fflush(stdout);
                    // fflush(stderr);
                    if (strcmp(strtok(input, " \n\r"), "e") == 0) {
                        break;
                    }
                    printf("Ready to wb buffer %s\n", buffer_main);
                    strcat(buffer_main, "\n");
                    send(connfd, buffer_main, strlen(buffer_main), 0);

                    // char buff[MAX_BUFFER_LENGTH];
                    // memset(buff, 0, MAX_BUFFER_LENGTH);
                    // sprintf(buff, "%d q", client_id);
                    // // write(pipefd[1], "q", 1);
                    // printf("in cliend %d have buff %s\n", client_id, buff);
                    // write(pipefd[1], buff, strlen(buff));
                    // memset(buff, 0, MAX_BUFFER_LENGTH);
                    // ssize_t numRead = read(wbfd[0], buff, sizeof(buff) - 1);
                    // buff[numRead] = '\0';
                    // printf("in client %d receive buf %s\n", client_id, buff);
                    // memset(info, 0, MAX_BUFFER_LENGTH);
                    // snprintf(info, sizeof(info), "\033[1;32mMini-FS\033[0m:\033[1;34m%s\033[0m$ ", buff);
                    // // printf("info is %s\n", info);
                    // send(connfd, info, strlen(info), 0);
                    // // fflush(stdout);
                    // memset(input, 0, MAX_BUFFER_LENGTH);
                    // int recv_len = recv(connfd, input, MAX_BUFFER_LENGTH - 1, 0);
                    // if (recv_len < 0) {
                    //     perror("Receive failed");
                    //     exit(-1);
                    // }
                    // printf("receive input %s\n", input);
                    // if (strcmp(strtok(input, " \n\r"), "e") == 0) {
                    //     break;
                    // }
                    // // fflush(stdout);
                    // // memset(buffer_main, 0, MAX_BUFFER_LENGTH);
                    // // parseCmd(input);
                    // // fflush(stdout);
                    // // fflush(stderr);
                    // memset(buff, 0, MAX_BUFFER_LENGTH);
                    // sprintf(buff, "%d %s", client_id, input);
                    // // write(pipefd[1], input, strlen(input));
                    // write(pipefd[1], buff, strlen(buff));
                    // memset(buff, 0, MAX_BUFFER_LENGTH);
                    // numRead = read(wbfd[0], buff, sizeof(buff) - 1);
                    
                    // printf("Ready to wb buffer %s\n", buff);
                    // strcat(buff, "\n");
                    // send(connfd, buff, strlen(buff), 0);
                }
                close(connfd);
            //     close(connfd);
            //     close(pipefd[1]);
            //     close(pipefd[0]);
            //     close(wbfd[0]);
            //     close(wbfd[1]);
            //     close(listenfd);
            //     // exit(0);
            // } else {
            //     close(connfd);
            //     close(pipefd[1]);
            //     close(pipefd[0]);
            //     close(wbfd[0]);
            //     close(wbfd[1]);
            // }
        }
    // } else {
    //     close(pipefd[1]);
    //     close(wbfd[0]);
        
    //     while (1) {
    //         char buff[MAX_BUFFER_LENGTH];
    //         memset(buff, 0, MAX_BUFFER_LENGTH);
    //         ssize_t numRead = read(pipefd[0], buff, sizeof(buff) - 1);
    //         buff[numRead] = '\0';
    //         printf("buff in parent proc is %s\n", buff);
    //         parseCmd(buff);
    //         printf("Ready to wb buffer in parent proc %s\n", buffer_main);
    //         if (strlen(buffer_main) == 0) {
    //             strcat(buffer_main, " ");
    //         }
    //         write(wbfd[1], buffer_main, strlen(buffer_main));
    //     }

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

int check_permission(uint16_t mask, int inode_id_cur) {
    if (client_id == 0) {
        // * root has super permission
        return 1;
    }

    uint16_t perm = inode_table[inode_id_cur]._permission;
    uint16_t indx = inode_table[inode_id_cur]._ownerID;
    printf("Owned by %d, visitor is %d\n", indx, client_id);
    uint16_t read_mask = mask & (1 << 2);
    uint16_t write_mask = mask & (1 << 1);
    uint16_t exec_mask = mask & 1;
    if (indx == client_id) {
        if (read_mask && !(perm & (1 << 2))) {
            return 0;
        }
        if (write_mask && !(perm & (1 << 1))) {
            return 0;
        }
        if (exec_mask && !(perm & (1 << 0))) {
            return 0;
        }
    } else {
        if (read_mask && !(perm & (1 << 5))) {
            return 0;
        }
        if (write_mask && !(perm & (1 << 4))) {
            return 0;
        }
        if (exec_mask && !(perm & (1 << 3))) {
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
    client_id = 0;
    for (int i = 1; i < MAX_INODE_NUM; ++i) {
        // printf("Init inode %d\n", i);
        init_inode(&inode_table[i], 0, 0, 0, i, UINT16_MAX, 0, 0);
    }
    memset(path_name, 0, MAX_BUFFER_LENGTH);
    strcpy(path_name, "/");
    initialized = 1;
    user_count = 1;
    User root;
    memset(root._user_id, 0, sizeof(root._user_id));
    strcpy(root._user_id, "ROOT");
    memset(root._user_key, 0, sizeof(root._user_key));
    strcpy(root._user_key, "");
    memset(user, 0, sizeof(user));
    memcpy(user, &root, sizeof(root));
    write1(sockfd, 0, 131, sizeof(user), user);
    printf("Success Init\n");
    // char info[MAX_BUFFER_LENGTH];
    // snprintf(info, sizeof(info), "%s", path_name);
    // send(sockfd, info, strlen(info), 0);
}

void lsOp() {
    if (!initialized) {
        perror("File system not initialized.");
        return;
    }
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    if (check_permission(4, cur_inode_id) == 0) {
        strcat(buffer_main, "Permission denied. ls needs read permission.");
        return;
    }
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
    // send(sockfd, temp, strlen(temp), 0);
    // exit(1);
}

void mkOp(char* name) {
    printf("mkOp called\n");
    printf("Client Op is %d\n", client_id);
    if (!initialized) {
        // printf("File system not initialized.\n");
        perror("File system not initialized.");
        return;
    }
    if (!validate_name(name)) {
        perror("Invalid name.");
        return;
    }
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    if (check_permission(3, cur_inode_id) == 0) {
        strcat(buffer_main, "Permission denied. mk needs write and exec permission.");
        return;
    }

    // * In `dir_add_entry`, we have checked whether a file with same name exists.
    int res = dir_add_entry(&inode_table[cur_inode_id], client_id, name, 0);
    printf("res is %d\n", res);
    
    if (res == 0) {
        strcat(buffer_main, "File has been created before.");
    } else if (res == 1) {
        strcat(buffer_main, "Successfully make a new file named ");
        strcat(buffer_main, name);
    }
    printf("buffer main in mkop is %s\n", buffer_main);
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
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);

    if (check_permission(3, cur_inode_id) == 0) {
        strcat(buffer_main, "Permission denied. mkdir needs write and exec permission.");
        return;
    }

    // * In `dir_add_entry`, we have checked whether a file with same name exists.
    int res = dir_add_entry(&inode_table[cur_inode_id], client_id, name, 1);
    
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
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    if (check_permission(2, cur_inode_id) == 0) {
        strcat(buffer_main, "Permission denied. rm needs write permission.");
        return;
    }

    int res = dir_del_entry(&inode_table[cur_inode_id], client_id, name);
    
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
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);

    if (check_permission(3, cur_inode_id) == 0) {
        strcat(buffer_main, "Permission denied. rmdir needs write and exec permission.");
        return;
    }

    int res = dir_del_entry_recursive(&inode_table[cur_inode_id], client_id, name);
    
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
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    // if (check_permission(1) == 0) {
    //     strcat(buffer_main, "Permission denied. cd needs exec permission.");
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
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    
    int file_inode_id = dir_check_existence(&inode_table[cur_inode_id], 0, name);
    if (file_inode_id == -1) {
        perror("File not found.");
        return;
    }

    if (check_permission(4, file_inode_id) == 0) {
        strcat(buffer_main, "Permission denied. cat needs read permission.");
        return;
    }
   
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
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);

    int file_inode_id = dir_check_existence(&inode_table[cur_inode_id], 0, file_name);
    if (file_inode_id == -1) {
        perror("File not found.");
        return;
    }

    if (check_permission(2, file_inode_id) == 0) {
        strcat(buffer_main, "Permission denied. write needs write permission.");
        return;
    }

    file_write(&inode_table[file_inode_id], client_id, data);
    
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

    if (check_permission(6, file_inode_id) == 0) {
        strcat(buffer_main, "Permission denied. insert needs read and write permission.");
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
    if (check_permission(6, file_inode_id) == 0) {
        strcat(buffer_main, "Permission denied. delete needs read and write permission.");
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

void qOp() {
    printf("qOp called\n");
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    strcat(buffer_main, path_name);
}

void crOp(char* name, char* key) {
    if (!initialized) {
        perror("File system not initialized.");
        return;
    }
    if (!validate_name(name)) {
        perror("Invalid name.");
        return;
    }
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    if (cur_inode_id != 0) {
        strcat(buffer_main, "You can only create user when you are in / path");
        return;
    } 
    if (client_id != 0) {
        strcat(buffer_main, "Permission denied. cr needs super permission.");
        return;
    }
    User temp[8];
    read1(sockfd, 0, 131);
    memcpy(temp, buffer[0], sizeof(temp));

    for (int i = 0; i < 8; ++i) {
        if (strlen(temp[i]._user_id) == 0) {
            user_count = i;
            break;
        }
    }

    if (user_count >= 6) {
        strcat(buffer_main, "Exceed maximum clients. At most 5!");
        return;
    }

    for (int i = 0; i < user_count; ++i) {
        if (strcmp(temp[i]._user_id, name) == 0) {
            strcat(buffer_main, "User has been created before.");
            return;
        }
    }
    
    User cur_user;
    memset(cur_user._user_id, 0, sizeof(cur_user._user_id));
    strcpy(cur_user._user_id, name);
    memset(cur_user._user_key, 0, sizeof(cur_user._user_key));
    strcpy(cur_user._user_key, key);
    // * wb

    memcpy(temp + user_count, &cur_user, sizeof(cur_user));
    write1(sockfd, 0, 131, sizeof(temp), temp);
    user_count += 1;
    client_id = user_count - 1;
    printf("temp[user_count - 1] id is %s, key is %s\n", temp[user_count - 1]._user_id, temp[user_count - 1]._user_key);
    strcat(buffer_main, "Successfully create user ");
    strcat(buffer_main, name);
    mkdirOp(name);
    client_id = 0;
}

void login(char* name, char* key) {
    if (!initialized) {
        perror("File system not initialized.");
        return;
    }
    if (!validate_name(name)) {
        perror("Invalid name.");
        return;
    }
    if (cur_inode_id != 0) {
        strcat(buffer_main, "You can only login when you are in / path");
        return;
    }
    memset(buffer_main, 0, MAX_BUFFER_LENGTH);
    User temp[8];
    read1(sockfd, 0, 131);
    memcpy(temp, buffer[0], sizeof(temp));
    for (int i = 0; i < 8; ++i) {
        if (strlen(temp[i]._user_id) == 0) {
            user_count = i;
            break;
        }
    }
    printf("user count is %d\n", user_count);
    for (int i = 0; i < user_count; ++i) {
        printf("temp id is %s, key is %s\n", temp[i]._user_id, temp[i]._user_key);
        if (strcmp(temp[i]._user_id, name) == 0 && strcmp(temp[i]._user_key, key) == 0) {
            client_id = i;
            cdOp(name);
            strcat(buffer_main, "\nSuccessfully login as ");
            strcat(buffer_main, name);
            return;
        }
    }
    strcat(buffer_main, "Fail to login.");
}

void chmodOp(char* name, uint16_t new_perm) {
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
    if (client_id != 0 && inode_table[file_inode_id]._ownerID != client_id) {
        strcat(buffer_main, "Permission denied. chmod needs super permission or you need to be the owner.");
        return;
    }

    if (check_permission(2, file_inode_id) == 0) {
        strcat(buffer_main, "Permission denied. chmod needs write permission.");
        return;
    }
    inode_table[file_inode_id]._permission = new_perm;
    strcat(buffer_main, "Successfully change the permission of ");
    strcat(buffer_main, name);
    write_inode_to_disc(&inode_table[file_inode_id], client_id, file_inode_id);
}

void parseCmd(char* command) {
    if (strlen(command) == 0) {
        return;
    }
    char* token = strtok(command, " \n\r");
    char** argv = malloc(MAX_ARG * sizeof(char*));
    int i = 0;
    while (token != NULL) {
        printf("token is %s\n", token);
        argv[i++] = token;
        token = strtok(NULL, " \n\r");
    }
    argv[i] = NULL;
    int id = atoi(argv[0]);
    client_id = id;
    
    for (int j = 1; j < i; ++j) {
        argv[j - 1] = argv[j];
    }
    --i;
    argv[i] = NULL;
    if (i == 1) {
        if (strcmp(argv[0], "f") == 0) {
            fOp();
        } else if (strcmp(argv[0], "ls") == 0) {
            lsOp();
        } else if (strcmp(argv[0], "e") == 0) {
            eOp();
        } else if (strcmp(argv[0], "q") == 0) {
            qOp();
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
    } else if (i == 3) {
        if (strcmp(argv[0], "cr") == 0) {
            crOp(argv[1], argv[2]);
        } else if (strcmp(argv[0], "login") == 0) {
            login(argv[1], argv[2]);
        } else if (strcmp(argv[0], "chmod") == 0) {
            chmodOp(argv[1], atoi(argv[2]));
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