/*
    * Superblock struct and related operation functions
*/
#include "superblock.h"
#include "wrapper.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int sockfd;
Superblock spbk;

void init_spbk(int block_size, int fd) {
    int real_block_size = (MAX_BLOCK_NUM >= block_size) ? block_size : MAX_BLOCK_NUM;
    if (real_block_size <= RESERVE_BLOCK_NUM) {
        printf("Block size is too small.\n");
        exit(-1);
    }
    // printf("Block size: %d\n", real_block_size);
    spbk._inode_count = 0;
    spbk._block_count = RESERVE_BLOCK_NUM;
    spbk._vacant_inode_count = MAX_INODE_NUM;
    spbk._vacant_block_count = real_block_size - RESERVE_BLOCK_NUM;
    spbk._root_inode_id = 0;
    memset(spbk._inode_bitmap, 0, sizeof(spbk._inode_bitmap));
    memset(spbk._block_bitmap, 0, sizeof(spbk._block_bitmap));
    for (int i = 0; i <= 3; ++i) {
        spbk._block_bitmap[i] = -1;
    }
    uint32_t mask = (1 << 31) | (1 << 30) | (1 << 29);
    spbk._block_bitmap[4] |= mask;
    sockfd = fd;
    // printf("fd = %d\n", sockfd);
    update_spbk();
}

void update_spbk() {
    char data[BLOCK_SIZE * 3 + 10];
    memcpy(data, &spbk, sizeof(Superblock));
    // printf("finish cp spbk, size is %d\n", strlen(data));
    // print_spbk();
    write1(sockfd, 0, 0, sizeof(Superblock), data);
}

void print_spbk() {
    printf("inode_count: %d\n", spbk._inode_count);
    printf("block_count: %d\n", spbk._block_count);
    printf("vacant_inode_count: %d\n", spbk._vacant_inode_count);
    printf("vacant_block_count: %d\n", spbk._vacant_block_count);
    printf("root_inode_id: %d\n", spbk._root_inode_id);
    printf("inode_bitmap: ");
    for (int i = 0; i < INODE_BITMAP_SIZE; ++i) {
        printf("%u ", spbk._inode_bitmap[i]);
    }
    printf("\nblock_bitmap: ");
    for (int i = 0; i < BLOCK_BITMAP_SIZE; ++i) {
        printf("%u ", spbk._block_bitmap[i]);
    }
    printf("\n");
}

int alloc_block() {
    if (spbk._vacant_block_count == 0) {
        return -1;
    }
    int blockID = -1;
    for (int i = 4; i < BLOCK_BITMAP_SIZE; ++i) {
        for (int j = 31; j >= 0; --j) {
            if ((spbk._block_bitmap[i] >> j) & 1) continue;
            else {
                blockID = i * 32 + 32 - j;
                spbk._block_bitmap[i] |= (1 << j);
                spbk._block_count++;
                spbk._vacant_block_count--;
                update_spbk();
                char buf[BLOCK_SIZE];
                memset(buf, 0, sizeof(buf));
                write1(sockfd, 0, blockID, BLOCK_SIZE, buf);
                return blockID;
            }
        }
    }
    // * Never used
    return blockID;
}

int gc_block(uint16_t blockID, uint16_t clientID) {
    if (blockID < RESERVE_BLOCK_NUM || blockID >= MAX_BLOCK_NUM) {
        return -1;
    }
    int c = blockID / 32;
    int r = blockID % 32;
    if (((spbk._block_bitmap[c] >> (31 - r)) & 1) == 0) {
        return 1;
    }
    spbk._block_count--;
    spbk._vacant_block_count++;
    spbk._block_bitmap[c] ^= (1 << (31 - r));
    write1(sockfd, clientID, blockID, 0, "");
    update_spbk();
    return 1;
}   