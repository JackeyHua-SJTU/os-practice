#include <unistd.h>
#include <stdio.h>
#include <time.h>

#define BUF_SIZE 100

int main(int argc, char* argv[]) {
    const char* src_name = argv[1];
    const char* dst_name = argv[2];

    clock_t start = clock();

    if (fork() == 0) {
        int p[2];
        pipe(p);
        char buf[BUF_SIZE];

        if (fork() == 0) {
            printf("Child started copying.\n");
            FILE* dst;
            dst = fopen(dst_name, "w+");
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
            close(p[0]);
            size_t read;
            while ((read = fread(buf, sizeof(char), BUF_SIZE, src)) > 0) {
                write(p[1], buf, read);
            }
            close(p[1]);
            fclose(src);
        }

    } else {
        wait(0);
        printf("Child has finished copying.\n");
        clock_t end = clock();
        double time_spent = ((double)end - start) / CLOCKS_PER_SEC;
        printf("Time spent: %f\n", time_spent);
    }

    return 0;
}