#include <stdio.h>
#include <time.h>

#define BUF_SIZE 100

void copy(FILE*, FILE*);

int main(int argc, char* argv[]) {
    clock_t start = clock();
    FILE *src, *dst;
    const char* src_name = argv[1];
    const char* dst_name = argv[2];
    src = fopen(src_name, "r");
    dst = fopen(dst_name, "w+");
    copy(src, dst);
    printf("Successful copy\n");
    clock_t end = clock();
    double time_spent = ((double)end - start) / CLOCKS_PER_SEC;
    printf("Time spent: %f\n", time_spent);
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