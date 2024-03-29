#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAX_PATH 1024
#define MAX_NAME 1024
#define MAX_INPUT 1024
#define MAX_ARGS 1024

/* Special commands. */
const char* CD = "cd";  // cd should change the current working directory, not in a child process.
const char* EXIT = "exit";  // exit should terminate the shell.
const char* PWD = "pwd";
const char* WC = "wc";
const char* CAT = "cat";
const char* LS = "ls";
const char* RM = "rm";

void init_name_and_path(char[], char[]);
int parse_input(char[], char**);
void exec(char**, int, int);

char wd[MAX_PATH];  // this is the path of this file, intended to find the exe even if we cd to a different directory

int main() {
    char name[MAX_NAME];
    char path[MAX_PATH];
    char input[MAX_INPUT];
    
    getcwd(wd, MAX_PATH);
    init_name_and_path(name, path);
    
    while (1) {
        /* change the color of info into what real bash is like */
        printf("\033[1;32m%s\033[0m:\033[1;34m%s\033[0m$ ", name, path);
        fgets(input, MAX_INPUT, stdin);
        char** argv = malloc(MAX_ARGS * sizeof(char*));
        
        int argc = parse_input(input, argv);
        
        if (argc == 0) {
            continue;
        } else if (strcmp(argv[0], EXIT) == 0) {
            break;
        } else if (strcmp(argv[0], CD) == 0) {
            int ret;
            if (argc == 1) {
                /* cd with default value will go to ~ dir */
                ret = chdir(getenv("HOME"));
            } else {
                ret = chdir(argv[1]);
            }
            if (ret == -1) {
                perror("cd");
            } else {
                init_name_and_path(name, path);
            }
        } else {
            exec(argv, 0, argc - 1);
        }
        free(argv);   
    }

    return 0;
}

void exec(char** argv, int lb, int rb) {
    if (lb < 0 || rb < 0 || lb > rb) {
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
    /* possible_right = -1 means there is no pipe, run it */
    if (possible_right == -1) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(-1);
        } else if (pid == 0) {
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
            wait(0);
        }
    } else { 
        /* There is a pipe, and [lb, possible_right] is the left part of the pipe */
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
            exec(argv, possible_right + 1, rb);
            /* We should recover the STDIN file descriptor. 0 now is the read side of a pipe. */
            close(0);
            dup(copy);
            close(copy);
        }
    } 
}


int parse_input(char input[], char** argv) {
    //! \n\r 是Linux下的回车键 而非\n
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