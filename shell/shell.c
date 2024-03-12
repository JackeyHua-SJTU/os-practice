#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAX_PATH 1024
#define MAX_NAME 1024
#define MAX_INPUT 1024
#define MAX_ARGS 1024

void init_name_and_path(char[], char[]);
int parse_input(char[], char**);
void exec(char**, int, int);

// TODO: support pipe
// Feat: 1. run single shell command
//       2. mimic shell colors

int main() {
    char name[MAX_NAME];
    char path[MAX_PATH];
    char input[MAX_INPUT];
    init_name_and_path(name, path);
    
    // TODO: fix bug in while(1)
    while (1) {
        /* change the color of info into what real bash is like */
        printf("\033[1;32m%s\033[0m:\033[1;34m%s\033[0m$ ", name, path);
        fgets(input, MAX_INPUT, stdin);
        char** argv = malloc(MAX_ARGS * sizeof(char*));
        
        int argc = parse_input(input, argv);
        
        // if (fork() == 0) {
        //     /* absolute path for shell command */
        //     char* bin = malloc(MAX_NAME * sizeof(char));
        //     sprintf(bin, "/bin/%s", argv[0]);
        //     execv(bin, argv);
        //     free(bin);
        // } else {
        //     wait(0);
        // }
        exec(argv, 0, argc - 1);
        free(argv);   
    }

    return 0;
}

void exec(char** argv, int lb, int rb) {
    // valid range: [lb, rb]
    /* check whether there is a pipe */
    int possible_right = -1;
    for (int i = lb; i <= rb; i++) {
        if (strcmp(argv[i], "|") == 0) {
            possible_right = i;
            break;
        }
    }
    printf("possible_right: %d\n", possible_right);
    if (possible_right == -1) {
        if (fork() == 0) {
            execvp(argv[lb], argv + lb);
        } else {
            wait(0);
        }
    } else { 
        int p[2];
        pipe(p);
        if (fork() == 0) {
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
            execvp(argv[lb], argv + lb);
        } else {
            wait(0);
            close(p[1]);
            close(0);
            dup(p[0]);
            close(p[0]);
            exec(argv, possible_right + 1, rb);
        }
    } 
}


int parse_input(char input[], char** argv) {
    char* p = strchr(input, '\n');
    if (p != NULL) {
        *p = '\0';
    }
    char* token = strtok(input, " ");
    int i = 0;
    while (token != NULL) {
        argv[i++] = token;
        token = strtok(NULL, " ");
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