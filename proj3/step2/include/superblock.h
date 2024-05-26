#ifndef _SUPERBLOCK_H_
#define _SUPERBLOCK_H_

#include <stdint.h>

#define BLOCK_SIZE 256
#define MAX_INODE_NUM 1024
#define MAX_BLOCK_NUM 4096
#define INODE_BITMAP_SIZE 32
#define BLOCK_BITMAP_SIZE 128
#define RESERVE_BLOCK_NUM 131

// * We need 3 blocks to store the superblock
typedef struct {
    uint16_t _inode_count;
    uint16_t _block_count;
    uint16_t _vacant_inode_count;
    uint16_t _vacant_block_count;
    uint16_t _root_inode_id;
    uint32_t _inode_bitmap[INODE_BITMAP_SIZE];  // * 0 for vacant, from left to right
    uint32_t _block_bitmap[BLOCK_BITMAP_SIZE];
} Superblock;

extern Superblock spbk;

void init_spbk(int, int);

void update_spbk();

int alloc_block();

int gc_block(uint16_t, uint16_t);

#endif