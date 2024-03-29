#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

#define NO_ARGS 0
#define DASH_A 1
#define DASH_L 2

char* month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void list_files(const char *path, int parameters) {
    struct dirent *entry;
    DIR *dir = opendir(path);
    long total = 0;

    if (dir == NULL) {
        perror("Unable to open directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (parameters == NO_ARGS || parameters == DASH_L) {
            if (entry->d_name[0] == '.') {
                continue;
            }
        }
        if (parameters == DASH_L) {
            struct stat fileStat;

            if (stat(entry->d_name, &fileStat) < 0) {
                perror("Error getting file stats");
                continue;
            }

            struct passwd *pw = getpwuid(fileStat.st_uid);
            struct group  *gr = getgrgid(fileStat.st_gid);
            total += fileStat.st_blocks;

            // metadata
            printf("%s", (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
            printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
            printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
            printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
            printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
            printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
            printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
            printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
            printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
            printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");
            printf(" %ld ", fileStat.st_nlink);
            printf(" %ld ", fileStat.st_size);

            // Print user and group
            if (pw != 0) {
                printf(" %s", pw->pw_name);
            }

            if (gr != 0) {
                printf(" %s", gr->gr_name);
            }

            // data and time
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            int mon = tm->tm_mon;
            int day = tm->tm_mday;
            printf(" %s %d %d:%d ", month[mon], day, tm->tm_hour, tm->tm_min);
        }

        // file name
        printf("%s\n", entry->d_name);
    }
    // get the total `num` of ls -l
    if (parameters == DASH_L) {
        int p[2];
        pipe(p);
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error forking");
            return;
        } else if (pid == 0) {
            close(p[0]);
            dup2(p[1], 1);
            close(p[1]);
            execvp("ls", (char *[]){"ls", "-l", NULL});
        } else {
            wait(NULL);
            close(p[1]);
            char buffer[1024];
            FILE* stream = fdopen(p[0], "r");
            if (fgets(buffer, sizeof(buffer), stream) != NULL) {
                printf("%s", buffer);
            }
            fclose(stream);
        }
    }

    closedir(dir);
}

int main(int argc, char** argv) {
    /*
        * List all files in the current directory
        * Support `ls` `ls -a` `ls -l` `ls $path`
        ! Any other form will be considered invalid
    */
    // printf("argc: %d\n", argc);
    // for (int i = 0; i < argc; i++) {
    //     printf("%s\n", argv[i]);
    // }

    if (argc > 2) {
        printf("Too many arguments\n");
        return 1;
    }
    if (argc == 1) {
        list_files(".", NO_ARGS);
        return 0;
    }

    if (strcmp(argv[1], "-a") == 0) {
        list_files(".", DASH_A);
    } else if (strcmp(argv[1], "-l") == 0) {
        list_files(".", DASH_L);
    } else {
        list_files(argv[1], NO_ARGS);
        // printf("Invalid argument\n");
    }
    
    return 0;
}