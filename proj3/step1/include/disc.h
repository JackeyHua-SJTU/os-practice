/*
    * Disc Struct
*/
#ifndef _DISC_H_
#define _DISC_H_

#include <stdio.h>

#define BLOCK_SIZE 256
#define MAX_ARGS 128

typedef struct {
    int _numCylinder;               // * # of cylinders
    int _numSectorPerCylinder;      // * # of sector per cylinder
    int _fd;                        // * file descriptor
    int _prev_cylinder;             // * previous cylinder
    double _trackDelay;             // * time delay moving between tracks
    char* _file;                    // * file name
    char* _discfile;                // * disc file mapped to the current file
    FILE* _fp;                      // * file pointer
} Disc;

void constructor(int numCylinder, int numSectorPerCylinder, double trackDelay, char* file, Disc* disc);

void decontructor(Disc* disc);

void parseCmdInput(Disc* disc, char* cmd);

void queryOp(Disc* disc);

void readOp(Disc* disc, int cylinderID, int sectorID);

void readOp_client(Disc* disc, int blockID);

void writeOp(Disc* disc, int cylinderID, int sectorID, int len, char* data);

void writeOp_client(Disc* disc, int blockID, int len, char* data);

#endif