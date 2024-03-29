#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Missing parameters\n");
        exit(-1);
    } else if (argc > 3) {
        printf("Too many parameters\n");
        exit(-1);
    } 

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
        // we do not want the output of mycopy
        close(1);
        execl("./MyCopy", "MyCopy", argv[1], argv[2], NULL);
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
        printf("Fork Copy takes : %f ms\n", time_spent);
        #elif TIME2 
        gettimeofday(&end, &tz);
        long seconds = end.tv_usec - start.tv_usec;
        printf("Fork Copy takes : %f ms \n", seconds / 1000.0);
        #endif
    }

    return 0;
}