#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>

#define BUF_SIZE 100

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Missing parameters\n");
        exit(-1);
    } else if (argc > 3) {
        printf("Too many parameters\n");
        exit(-1);
    } 

    const char* src_name = argv[1];
    const char* dst_name = argv[2];

    #ifdef TIME1
    clock_t start = clock();
    #elif TIME2
    struct timeval start, end;
    struct timezone tz;
    struct tm *tm;
    gettimeofday(&start, &tz);
    #endif

    pid_t pid = fork();
    if (pid < 0) {
        printf("Fork failed\n");
        exit(-1);
    } else if (pid == 0) {
        int p[2];
        if (pipe(p) < 0) {
            printf("Pipe failed\n");
            exit(-1);
        }
        char buf[BUF_SIZE];
        pid_t pid2 = fork();
        if (pid2 < 0) {
            printf("Fork failed\n");
            exit(-1);
        } else if (pid2 == 0) {
            FILE* dst;
            dst = fopen(dst_name, "w+");
            if (dst == NULL) {
                perror("Error opening destination file");
                exit(-1);
            }
            close(p[1]);
            size_t r;
            while ((r = read(p[0], buf, BUF_SIZE)) > 0) {
                fwrite(buf, sizeof(char), r, dst);
            }
            close(p[0]);
            fclose(dst);
        } else {
            FILE* src;
            src = fopen(src_name, "r");
            if (src == NULL) {
                perror("Error opening source file");
                exit(-1);
            }
            close(p[0]);
            size_t read;
            while ((read = fread(buf, sizeof(char), BUF_SIZE, src)) > 0) {
                write(p[1], buf, read);
            }
            close(p[1]);
            fclose(src);
        }

    } else {
        int status;
        wait(&status);
        if (status < 0) {
            printf("Error copying\n");
            exit(-1);
        }
        #ifdef TIME1
        clock_t end = clock();
        double time_spent = ((double)end - start) / CLOCKS_PER_SEC * 1000.0;
        printf("Pipe Copy takes : %f ms\n", time_spent);
        #elif TIME2 
        gettimeofday(&end, &tz);
        long seconds = end.tv_usec - start.tv_usec;
        printf("Pipe Copy takes : %f ms \n", seconds / 1000.0);
        #endif
    }

    return 0;
}