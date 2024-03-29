#include <stdio.h>

int main(int argc, char** argv) {
    // Does not accept any parameters
    // can only be called with `cat <file>`
    if (argc != 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("fopen");
        return 1;
    }

    int c;
    while ((c = fgetc(file)) != EOF) {
        putchar(c);
    }

    fclose(file);
    printf("\n");
    return 0;
}