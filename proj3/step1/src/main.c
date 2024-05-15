#include "disc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#define MAX_INPUT 256

int main(int argc, char** argv) {
    Disc d;
    if (argc != 5) {
        printf("Usage: [filename] [# of cylinders] [# of sectors per cyclinder] [track to track delay]");
        exit(-1);
    }
    constructor(atoi(argv[2]), atoi(argv[3]), strtod(argv[4], NULL), argv[1], &d);
    char input[MAX_INPUT];
    while (1) {
        memset(input, 0, sizeof(input));
        fgets(input, sizeof(input), stdin);
        parseCmdInput(&d, input);
    }

}