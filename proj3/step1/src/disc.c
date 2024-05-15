#include "disc.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void constructor(int numCylinder, int numSectorPerCylinder, double trackDelay, char* file, Disc* disc) {
    disc->_file = file;
    disc->_numCylinder = numCylinder;
    disc->_numSectorPerCylinder = numSectorPerCylinder;
    disc->_trackDelay = trackDelay;
    disc->_fd = open(disc->_file, O_RDWR | O_CREAT);
    if (disc->_fd == -1) {
        printf("Error in creating or opening file %s. Check again.", disc->_file);
        exit(-1);
    }
    long FILESIZE = BLOCK_SIZE * disc->_numCylinder * disc->_numSectorPerCylinder;
    int result = lseek(disc->_fd, FILESIZE - 1, SEEK_SET);
    if (result == -1) {
        perror("Error calling lseek() to stretch the file.");
        close(disc->_fd);
        exit(-1);
    }
    result = write(disc->_fd, "", 1);
    if (result != 1) {
        perror("Error writing last byte of the file.");
        close(disc->_fd);
        exit(-1);
    }
    disc->_discfile = (char*) mmap(NULL, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, disc->_fd, 0);
    if (disc->_discfile == MAP_FAILED) {
        close(disc->_fd);
        perror("Error: Could not map file.");
        exit(-1);
    }
}

void decontructor(Disc* disc) {
    free(disc->_file);
    int result = munmap(disc->_discfile, BLOCK_SIZE * disc->_numCylinder * disc->_numSectorPerCylinder);
    if (result == -1) {
        perror("Error when unmmaping the memory space allocated to the file.");
        close(disc->_fd);
        exit(-1);
    }
    result = close(disc->_fd);
    if (result == -1) {
        perror("Error when closing the file descriptor.");
        exit(-1);
    }
}

void parseCmdInput(Disc* disc, char* cmd) {
    // printf("Input is %s\n", cmd);
    char* token = strtok(cmd, " \n");
    char** argv = malloc(MAX_ARGS * sizeof(char*));
    int i = 0;
    while (token != NULL) {
        argv[i++] = token;
        token = strtok(NULL, " \n");
    }
    argv[i] = NULL;
    // printf("i is %d\n", i);
    // for (int j = 0; j < i; ++j) {
    //     printf("argv[%d] is %s\n", j, argv[j]);
    // }
    switch (i) {
        // * In parsing, we need to make sure int type arguments do have its int type value
        // * Whether the value is valid will be left to specific dealing function.
        case 1:
            if (strcmp(argv[0], "I") != 0) {
                perror("Wrong types of command. Please input 'I'.");
                free(argv);
                exit(-1);
            } else {
                queryOp(disc);
            }
            break;
        case 3:
            if (strcmp(argv[0], "R") != 0) {
                perror("Wrong types of command. Please input 'R [cyclinder] [sector]'.");
                free(argv);
                exit(-1);
            } else {
                int cylinder = atoi(argv[1]), sector = atoi(argv[2]);
                if (cylinder == 0 && strcmp(argv[1], "0") != 0) {
                    perror("Invalid cyclinder number, check it out.");
                    free(argv);
                    exit(-1);
                }
                if (sector == 0 && strcmp(argv[2], "0") != 0) {
                    perror("Invalid sector number, check it out.");
                    free(argv);
                    exit(-1);
                }
                // printf("cylinder is %d, sector is %d\n", cylinder, sector);
                readOp(disc, cylinder, sector);
            }
            break;

        case 5:
            if (strcmp(argv[0], "W") != 0) {
                perror("Wrong types of command. Please input 'W [cyclinder] [sector] [length] [data]'.");
                free(argv);
                exit(-1);
            } else {
                int cylinder = atoi(argv[1]), sector = atoi(argv[2]), len = atoi(argv[3]);
                if (cylinder == 0 && strcmp(argv[1], "0") != 0) {
                    perror("Invalid cyclinder number, check it out.");
                    free(argv);
                    exit(-1);
                }
                if (sector == 0 && strcmp(argv[2], "0") != 0) {
                    perror("Invalid sector number, check it out.");
                    free(argv);
                    exit(-1);
                }
                if (len == 0 && strcmp(argv[3], "0") != 0) {
                    perror("Invalid len, check it out.");
                    free(argv);
                    exit(-1);
                }
                writeOp(disc, cylinder, sector, len, argv[4]);
            }
            break;

        default:
            perror("Wrong number of arguments.");
            free(argv);
            exit(-1);
    }
    free(argv);
}

void queryOp(Disc* disc) {
    printf("%d %d\n", disc->_numCylinder, disc->_numSectorPerCylinder);
}

void readOp(Disc* disc, int cylinderID, int sectorID) {
    char buf[BLOCK_SIZE + 1];
    if (cylinderID < 0 || cylinderID >= disc->_numCylinder || sectorID < 0 || sectorID >= disc->_numSectorPerCylinder) {
        printf("No\n");
        return;
    }
    // printf("discfile: %s\n", disc->_discfile);
    memcpy(buf, &disc->_discfile[BLOCK_SIZE * (cylinderID * disc->_numSectorPerCylinder + sectorID)], BLOCK_SIZE);
    printf("Yes %s\n", buf);
}

void writeOp(Disc* disc, int cylinderID, int sectorID, int len, char* data) {
    if (cylinderID < 0 || cylinderID >= disc->_numCylinder || sectorID < 0 || sectorID >= disc->_numSectorPerCylinder || len < 0 || len >= BLOCK_SIZE) {
        printf("No\n");
        return;
    }
    // TODO: set 0 
    memcpy(&disc->_discfile[BLOCK_SIZE * (cylinderID * disc->_numSectorPerCylinder + sectorID)], data, len);
    printf("Yes\n");
}