#include <unistd.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Missing parameters\n");
    }

    clock_t start = clock();

    if (fork() == 0) {
        printf("Child started copying.\n");
        execl("./MyCopy", "MyCopy", argv[1], argv[2], NULL);
    } else {
        wait(0);
        printf("Child has finished copying.\n");
        clock_t end = clock();
        double time_spent = ((double)end - start) / CLOCKS_PER_SEC;
        printf("Time spent: %f\n", time_spent);
    }

    return 0;
}