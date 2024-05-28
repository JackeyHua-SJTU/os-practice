#ifndef _INODE_H_
#define _INODE_H_

#include <stdint.h>
#include "superblock.h"

#define DIRECT_LINK 7

// * We need to modify the order to avoid byte aligning.
// * Padding is 2 bytes
// ! sizeof(Inode) = 32 Bytes
typedef struct {
    uint8_t _mode;                           // * 0 for file and 1 for dir
    uint8_t _direct_count;                   // * # of links associated with this file
    uint16_t _file_size;                     // * file size
    uint32_t _last_modified_timestamp;       // * timestamp
    uint16_t _index;                         // * current inode index
    uint16_t _fa_index;                      // * father inode index, -1 for not exist (65535)
    uint16_t _direct_block[DIRECT_LINK];     // * direct block link, store block id
    uint16_t _indirect_block;                // * indirect block link, one level
    uint16_t _permission;                    // * only low 6 bits are valid, owner & group permission
    uint16_t _ownerID;                       // * owner ID
} Inode;

extern Inode inode_table[MAX_INODE_NUM];

// * We need a root inode, like / in linux system
void init_root(int);

void update_inode_table();

// ! Inode-level function
// * Allocate and initialize an new inode, delete an inode, update an inode
int alloc_inode();

int gc_inode(Inode*, uint16_t);

void write_inode_to_disc(Inode*, uint16_t, int);

void init_inode(Inode*, uint8_t, uint8_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);

// ! Dir-level function
int dir_check_existence(Inode*, uint8_t, char*);

int dir_add_entry(Inode*, uint16_t, char*, uint8_t);

int dir_del_entry(Inode*, uint16_t, char*);

int dir_del_entry_recursive(Inode*, uint16_t, char*);

void dir_ls(Inode*, char*);

// ! File-level function
void file_read(Inode*, uint16_t, char*);

void file_write(Inode*, uint16_t, char*);

#endif