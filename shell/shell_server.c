#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>

#define MAX_CLIENTS 5
#define MAX_BUFFER 1024
#define MAX_PATH 1024
#define MAX_NAME 1024
#define MAX_INPUT 1024
#define MAX_ARGS 1024


/* Special commands. */
const char* CD = "cd";  // cd should change the current working directory, not in a child process.
const char* EXIT = "exit";  // exit should terminate the shell.
/* self implemented shell commands with limited parameters supported */
const char* PWD = "pwd";
const char* WC = "wc";
const char* CAT = "cat";
const char* LS = "ls";
const char* RM = "rm";

void init_name_and_path(char[], char[]);
int parse_input(char[], char**);
void exec(char**, int, int, int);

char wd[MAX_PATH];  // this is the path of this file, intended to find the executable even if we cd to a different directory


int main(int argc, char** argv) {
    getcwd(wd, MAX_PATH);
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
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
    int i = 0;
    while (1) {
        socklen_t client_len = sizeof(client_addr);

        clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
        ++i;
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
        // Handle the client connection in a new process
        if (pid == 0) {
            printf("New child process (seq #%d)\n", i);
        
            close(sockfd);  // The child process does not need the listening socket

            char name[MAX_NAME];
            char path[MAX_PATH];
            char input[MAX_INPUT];
            init_name_and_path(name, path);
    
            while (1) {
                /* change the color of info into what real bash is like */
                char info[MAX_NAME + MAX_PATH + 10];
                snprintf(info, sizeof(info), "\033[1;32m%s\033[0m:\033[1;34m%s\033[0m$ ", name, path);
                write(clientfd, info, strlen(info));
                ssize_t bytes = read(clientfd, input, MAX_INPUT - 1);
                // input[bytes] = '\0';
                char** arg = malloc(MAX_ARGS * sizeof(char*));
        
                int n = parse_input(input, arg);

                arg[n] = NULL;
                if (n == 0) {
                    continue;
                } else if (strcmp(arg[0], EXIT) == 0) {
                    close(clientfd);
                    close(sockfd);
                    break;
                } else if (strcmp(arg[0], CD) == 0) {
                    int ret;
                    if (n == 1) {
                        /* cd with default value will go to ~ dir */
                        ret = chdir(getenv("HOME"));
                    } else {
                        ret = chdir(arg[1]);
                    }
                    if (ret == -1) {
                        dup2(clientfd, 1);
                        dup2(clientfd, 2);
                        perror("cd");
                    } else {
                        init_name_and_path(name, path);
                    }
                } else {
                    exec(arg, 0, n - 1, clientfd);
                }
                free(arg);   
                // flush the input array
                memset(input, 0, MAX_INPUT);
            }

            close(clientfd);  // Close the connection when done
            exit(EXIT_SUCCESS);
        } else {
            close(clientfd);  // The parent process does not need the client socket
        }
    }
    close(sockfd);
    return 0;
}

void exec(char** argv, int lb, int rb, int clientfd) {
    if (lb < 0 || rb < 0 || lb > rb) {
        dup2(clientfd, 1);
        printf("Invalid parameters.\n");
        return;
    }

    // valid range: [lb, rb]
    /* check whether there is a pipe */
    int possible_right = -1;
    for (int i = lb; i <= rb; i++) {
        if (strcmp(argv[i], "|") == 0) {
            possible_right = i;
            break;
        }
    }
    // printf("possible_right: %d\n", possible_right);
    if (possible_right == -1) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(-1);
        } else if (pid == 0) {
            // close(1);
            // dup(clientfd);
            // close(2);
            // dup(clientfd);
            dup2(clientfd, 1);
            dup2(clientfd, 2);
            
            int ret;
            if (strcmp(argv[lb], PWD) == 0 || strcmp(argv[lb], WC) == 0 || strcmp(argv[lb], CAT) == 0 || strcmp(argv[lb], LS) == 0 || strcmp(argv[lb], RM) == 0) {
                char concat[MAX_PATH + 10];
                sprintf(concat, "%s/%s", wd, argv[lb]);
                argv[lb] = concat;
                // printf("argv[lb]: %s\n", argv[lb]);
                ret = execv(argv[lb], argv + lb);
            } else {
                ret = execvp(argv[lb], argv + lb);
            }
            if (ret == -1) {
                perror("execvp");
                exit(-1);
            }
        } else {
            wait(0);
            return;
        }
    } else { 
        int p[2];
        pipe(p);
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(-1);
        } else if (pid == 0) {
            close(p[0]);
            close(1);
            dup(p[1]);
            close(p[1]);
            // char** new_argv[possible_right - lb + 1];
            // for (int i = lb; i < possible_right; i++) {
            //     *new_argv[i - lb] = argv[i];
            // }
            // new_argv[possible_right - lb] = NULL;
            argv[possible_right] = NULL;
            int ret;
            if (strcmp(argv[lb], PWD) == 0 || strcmp(argv[lb], WC) == 0 || strcmp(argv[lb], CAT) == 0 || strcmp(argv[lb], LS) == 0 || strcmp(argv[lb], RM) == 0) {
                char concat[MAX_PATH + 10];
                sprintf(concat, "%s/%s", wd, argv[lb]);
                argv[lb] = concat;
                ret = execv(argv[lb], argv + lb);
            } else {
                ret = execvp(argv[lb], argv + lb);
            }
            if (ret == -1) {
                perror("execvp");
                exit(-1);
            }
        } else {
            int status;
            wait(&status);
            if (status != 0) {
                return;
            }
            int copy = dup(0);
            close(p[1]);
            close(0);
            dup(p[0]);
            close(p[0]);
            dup2(clientfd, 1);
            dup2(clientfd, 2);
            exec(argv, possible_right + 1, rb, clientfd);
            /* We should recover the STDIN file descriptor. 0 now is the read side of a pipe. */
            close(0);
            dup(copy);
            close(copy);
        }
    } 
}


int parse_input(char input[], char** argv) {
    char* token = strtok(input, " \n\r");
    int i = 0;
    while (token != NULL) {
        argv[i++] = token;
        token = strtok(NULL, " \n\r");
    }
    argv[i] = NULL;
    return i;
}

void init_name_and_path(char name[], char path[]) {
    char tmp[MAX_PATH];
    /* get current workding directory */
    getcwd(tmp, MAX_PATH);
    strcpy(path, tmp);
    memset(tmp, 0, MAX_PATH);

    /* get current user name */
    // getlogin_r(tmp, MAX_NAME);
    char* username = getenv("USER");
    // strcpy(name, username);
    // memset(tmp, 0, MAX_PATH);

    /* get current host name */
    gethostname(tmp, MAX_PATH);
    // strcat(name, "@");
    // strcat(name, tmp);
    sprintf(name, "%s@%s", username, tmp);
}