#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

#define BUF_SIZE 100

void copy(FILE*, FILE*);

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Missing parameters\n");
        exit(-1);
    } else if (argc > 3) {
        printf("Too many parameters\n");
        exit(-1);
    } 

    FILE *src, *dst;
    const char* src_name = argv[1];
    const char* dst_name = argv[2];
    src = fopen(src_name, "r");
    if (src == NULL) {
        perror("Error opening source file");
        exit(-1);
    }
    dst = fopen(dst_name, "w+");
    if (dst == NULL) {
        perror("Error opening destination file");
        fclose(src);
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
    copy(src, dst);
    printf("Successful copy\n");
    #ifdef TIME1
    clock_t end = clock();
    double time_spent = ((double)end - start) / CLOCKS_PER_SEC * 1000.0;
    printf("My Copy takes : %f ms \n", time_spent);
    #elif TIME2 
    gettimeofday(&end, &tz);
    long seconds = end.tv_usec - start.tv_usec;
    printf("My Copy takes : %f ms \n", seconds / 1000.0);
    #endif
    return 0;
}

void copy(FILE* src, FILE* dst) {
    char buf[BUF_SIZE];

    size_t read;

    while ((read = fread(buf, sizeof(char), BUF_SIZE, src)) > 0) {
        fwrite(buf, sizeof(char), read, dst);
    }

    fclose(src);
    fclose(dst);
}