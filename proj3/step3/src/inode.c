#include "inode.h"
#include "superblock.h"
#include "wrapper.h"
#include "entry.h"
#include <time.h>
#include <string.h>
#include <stdio.h>

static int sockfd;
Inode inode_table[MAX_INODE_NUM];

// TODO: Change the permission of every inode
// TODO: Check whether `strcat()` work as expected in file operation function

void init_root(int fd) {
    Inode root;
    memset(inode_table, 0, sizeof(inode_table));
    // * This is the root DIRECTORY.
    inode_table[0]._mode = 1;
    inode_table[0]._direct_count = 0;
    inode_table[0]._last_modified_timestamp = time(NULL);
    inode_table[0]._file_size = 0;
    inode_table[0]._index = 0;
    inode_table[0]._fa_index = UINT16_MAX;
    memset(inode_table[0]._direct_block, 0, sizeof(inode_table[0]._direct_block));
    inode_table[0]._indirect_block = 0;
    inode_table[0]._permission = -1;
    inode_table[0]._ownerID = 0;
    spbk._inode_bitmap[0] |= (1 << 31);
    sockfd = fd;
    printf("fd in inode.c is %d\n", sockfd);
    printf("Init root inode.\n");
    write_inode_to_disc(&inode_table[0], 0, 0);
    printf("Init root inode done.\n");
    update_spbk();
}

void update_inode_table() {
    char data[MAX_INODE_NUM * sizeof(Inode) + 10];
    int len = MAX_INODE_NUM * sizeof(Inode);
    memcpy(data, inode_table, len);
    // * [0, 2] block is used for superblock
    // * Inode table starts from block #3
    write1(sockfd, 0, 3, len, data);
}

int alloc_inode() {
    if (spbk._vacant_inode_count == 0) {
        return -1;
    }
    int inodeID = -1;
    for (int i = 0; i < INODE_BITMAP_SIZE; ++i) {
        for (int j = 31; j >= 0; --j) {
            if ((spbk._inode_bitmap[i] >> j) & 1) continue;
            else {
                inodeID = i * 32 + 32 - j;
                spbk._inode_bitmap[i] |= (1 << j);
                spbk._inode_count++;
                spbk._vacant_inode_count--;
                update_spbk();
                return inodeID;
            }
        }
    }
    // * Never used
    return inodeID;
}

// ! In this function, we just delete all block associated with this inode
// ! We have another function for dir type recursive deletion
int gc_inode(Inode* inode, uint16_t clientID) {
    if (inode->_index == 0) {
        return -1;
    }
    int c = inode->_index / 32;
    int r = inode->_index % 32;

    if (((spbk._inode_bitmap[c] >> (31 - r)) & 1) == 0) {
        return 1;
    }

    for (int i = 0; i < DIRECT_LINK; ++i) {
        gc_block(inode->_direct_block[i], clientID);
    }

    if (inode->_indirect_block != 0) {
        // * Have an indirect block
        read1(sockfd, clientID, inode->_indirect_block);
        uint16_t indirect_blockid[128];
        memcpy(indirect_blockid, buffer[clientID], BLOCK_SIZE);
        for (int i = 0; i < 128; ++i) {
            gc_block(indirect_blockid[i], clientID);
        }
        gc_block(inode->_indirect_block, clientID);        
    }

    spbk._inode_bitmap[c] ^= (1 << (31 - r));
    spbk._inode_count--;
    spbk._vacant_inode_count++;
    update_spbk();
    return 0;
}

int gc_inode_recursive(Inode* inode, uint16_t clientID) {
    if (inode->_mode == 0) return -1;
    int c = inode->_index / 32;
    int r = inode->_index % 32;
    if (((spbk._inode_bitmap[c] >> (31 - r)) & 1) == 0) {
        return 1;
    }
    Entry entry[BLOCK_SIZE / sizeof(Entry)];
    for (int i = 0; i < DIRECT_LINK; ++i) {
        if (inode->_direct_block[i] == 0) continue;
        read1(sockfd, clientID, inode->_direct_block[i]);
        memset(entry, 0, sizeof(entry));
        memcpy(entry, buffer[clientID], BLOCK_SIZE);
        for (int j = 0; j < BLOCK_SIZE / sizeof(Entry); ++j) {
            if (entry[j]._state == 0) {
                continue;
            } else {
                if (entry[j]._type == 1) {
                    gc_inode_recursive(&inode_table[entry[j]._inode_id], clientID);
                } else {
                    gc_inode(&inode_table[entry[j]._inode_id], clientID);
                }
            }
        }
    }
    spbk._inode_bitmap[c] ^= (1 << (31 - r));
    spbk._inode_count--;
    spbk._vacant_inode_count++;
    update_spbk();
    return 0;
}


void write_inode_to_disc(Inode* inode, uint16_t clientID, int index) {
    int blockID = index / 8 + 3;
    int offset = index % 8;
    Inode buf[8];
    read1(sockfd, clientID, blockID);
    memcpy(buf, buffer[clientID], BLOCK_SIZE);
    buf[offset] = *inode;
    // printf("buf is %s\n", buf);
    write1(sockfd, clientID, blockID, BLOCK_SIZE, (void*)buf);
}

void init_inode(Inode* inode, uint8_t mode, uint8_t direct_count, uint16_t file_size, uint16_t index, uint16_t fa_index, uint16_t permission, uint16_t ownerID) {
    inode->_mode = mode;
    inode->_direct_count = direct_count;
    inode->_file_size = file_size;
    inode->_last_modified_timestamp = time(NULL);
    inode->_index = index;
    inode->_fa_index = fa_index;
    memset(inode->_direct_block, 0, sizeof(inode->_direct_block));
    inode->_indirect_block = 0;
    inode->_permission = permission;
    inode->_ownerID = ownerID;
    write_inode_to_disc(inode, ownerID, index);
}

int dir_check_existence(Inode* inode, uint8_t mode, char* name) {
    if (inode->_mode != 1) {
        return -1;
    }
    Entry entry[BLOCK_SIZE / sizeof(Entry)];
    for (int i = 0; i < DIRECT_LINK; ++i) {
        if (inode->_direct_block[i] == 0) continue;
        memset(entry, 0, sizeof(entry));
        read1(sockfd, inode->_ownerID, inode->_direct_block[i]);
        memcpy(entry, buffer[inode->_ownerID], BLOCK_SIZE);
        for (int j = 0; j < BLOCK_SIZE / sizeof(Entry); ++j) {
            if (entry[j]._state == 0) continue;
            if (strcmp(entry[j]._name, name) == 0 && entry[j]._type == mode) {
                return entry[j]._inode_id;
            }
        }
    }
    return -1;
}

int dir_add_entry(Inode* inode, uint16_t clientID, char* name, uint8_t mode) {
    // * Make sure it is a DIR type inode
    if (inode->_mode != 1) {
        return -1;
    }
    if (dir_check_existence(inode, mode, name) != -1) return 0;
    // printf("Pass check\n");
    Entry entry[BLOCK_SIZE / sizeof(Entry)];
    char wb_buf[BLOCK_SIZE];
    // * We first check if there are vacant entry in allocated block
    int i = 0;
    for (; i < DIRECT_LINK; ++i) {
        if (inode->_direct_block[i] == 0) continue;
        read1(sockfd, clientID, inode->_direct_block[i]);
        memset(entry, 0, sizeof(entry));
        memcpy(entry, buffer[clientID], BLOCK_SIZE);
        for (int j = 0; j < BLOCK_SIZE / sizeof(Entry); ++j) {
            if (entry[j]._state == 1) continue;
            int inodeID = alloc_inode();
            if (inodeID == -1) {
                // * No more inode
                return -1;
            }
            entry[j]._inode_id = inodeID;
            entry[j]._state = 1;
            strcpy(entry[j]._name, name);
            entry[j]._type = mode;
            init_inode(&inode_table[inodeID], mode, 0, 0, inodeID, inode->_index, 7, clientID);
            inode->_last_modified_timestamp = time(NULL);
            memset(wb_buf, 0, BLOCK_SIZE);
            memcpy(wb_buf, entry, BLOCK_SIZE);
            // printf("before write1\n");
            write1(sockfd, clientID, inode->_direct_block[i], BLOCK_SIZE, wb_buf);
            // printf("pass write1\n");
            write_inode_to_disc(inode, clientID, inode->_index);
            // printf("pass write inode to disc\n");
            return 1;
        }
    }
    // * No vacant entry in allocated block
    // * Try allocate new vacant block
    i = 0;
    for (; i < DIRECT_LINK; ++i) {
        if (inode->_direct_block[i] != 0) continue;
        int blockID = alloc_block();
        if (blockID < 0) return -1;
        inode->_direct_block[i] = blockID;
        inode->_direct_count++;
        memset(entry, 0, sizeof(entry));
        int inodeID = alloc_inode();
        if (inodeID < 0) return -1;
        entry[0]._inode_id = inodeID;
        entry[0]._state = 1;
        strcpy(entry[0]._name, name);
        entry[0]._type = mode;
        init_inode(&inode_table[inodeID], mode, 0, 0, inodeID, inode->_index, 7, clientID);
        inode->_last_modified_timestamp = time(NULL);
        memset(wb_buf, 0, BLOCK_SIZE);
        memcpy(wb_buf, entry, BLOCK_SIZE);
        // printf("before write1\n");
        write1(sockfd, clientID, inode->_direct_block[i], BLOCK_SIZE, wb_buf);
        // printf("pass write1\n");
        write_inode_to_disc(inode, clientID, inode->_index);
        // printf("pass write inode to disc\n");
        return 1;
    }
    // TODO: Add indirect link check
}

int dir_del_entry(Inode* inode, uint16_t clientID, char* name) {
    if (inode->_mode != 1) return -1;
    int id = dir_check_existence(inode, 0, name);
    if (id == -1) return 0;
    Entry entry[BLOCK_SIZE / sizeof(Entry)];
    char wb_buf[BLOCK_SIZE];
    for (int i = 0; i < DIRECT_LINK; ++i) {
        if (inode->_direct_block[i] == 0) continue;
        memset(entry, 0, sizeof(entry));
        read1(sockfd, clientID, inode->_direct_block[i]);
        memcpy(entry, buffer[clientID], BLOCK_SIZE);
        for (int j = 0; j < BLOCK_SIZE / sizeof(Entry); ++j) {
            if (entry[j]._state == 0) continue;
            if (strcmp(entry[j]._name, name) == 0 && entry[j]._type == 0) {
                entry[j]._state = 0;
                memset(wb_buf, 0, BLOCK_SIZE);
                memcpy(wb_buf, entry, BLOCK_SIZE);
                write1(sockfd, clientID, inode->_direct_block[i], BLOCK_SIZE, wb_buf);
                gc_inode(&inode_table[id], clientID);
                inode->_last_modified_timestamp = time(NULL);
                write_inode_to_disc(inode, clientID, inode->_index);
                return 1;
            }
        }
    }
}

int dir_del_entry_recursive(Inode* inode, uint16_t clientID, char* name) {
    if (inode->_mode != 1) return -1;
    int id = dir_check_existence(inode, 1, name);
    if (id == -1) return 0;
    Entry entry[BLOCK_SIZE / sizeof(Entry)];
    char wb_buf[BLOCK_SIZE];
    for (int i = 0; i < DIRECT_LINK; ++i) {
        if (inode->_direct_block[i] == 0) continue;
        memset(entry, 0, sizeof(entry));
        read1(sockfd, clientID, inode->_direct_block[i]);
        memcpy(entry, buffer[clientID], BLOCK_SIZE);
        for (int j = 0; j < BLOCK_SIZE / sizeof(Entry); ++j) {
            if (entry[j]._state == 0) continue;
            if (strcmp(entry[j]._name, name) == 0 && entry[j]._type == 1) {
                entry[j]._state = 0;
                memset(wb_buf, 0, BLOCK_SIZE);
                memcpy(wb_buf, entry, BLOCK_SIZE);
                write1(sockfd, clientID, inode->_direct_block[i], BLOCK_SIZE, wb_buf);
                gc_inode_recursive(&inode_table[id], clientID);
                inode->_last_modified_timestamp = time(NULL);
                write_inode_to_disc(inode, clientID, inode->_index);
                return 1;
            }
        }
    }
}

void dir_ls(Inode* inode, char* ans) {
    if (inode->_mode == 0) return;
    char buf[64 * MAX_NAME_LEN + 256];
    Entry entry[BLOCK_SIZE / sizeof(Entry)];
    memset(buf, 0, sizeof(buf));
    for (int i = 0; i < DIRECT_LINK; ++i) {
        if (inode->_direct_block[i] == 0) continue;
        memset(entry, 0, sizeof(entry));
        read1(sockfd, inode->_ownerID, inode->_direct_block[i]);
        memcpy(entry, buffer[inode->_ownerID], BLOCK_SIZE);
        for (int j = 0; j < BLOCK_SIZE / sizeof(Entry); ++j) {
            if (entry[j]._state == 0) continue;
            if (entry[j]._type == 1) {
                strcat(buf, "\033[1;32m");
                strcat(buf, entry[j]._name);
                strcat(buf, "\033[0m ");
            } else {
                strcat(buf, entry[j]._name);
                strcat(buf, " ");
            }
        }
    }
    // strcat(buf, '\0');
    strcpy(ans, buf);
}

void file_read(Inode* inode, uint16_t clientID, char* ans) {
    if (inode->_mode == 1) return;
    char buf[BLOCK_SIZE + 1];
    memset(buf, 0, sizeof(buf));

    int last_block_size = inode->_file_size % BLOCK_SIZE;
    
    if (inode->_direct_count <= DIRECT_LINK) {
        for (int i = 0; i < inode->_direct_count; ++i) {
            if (inode->_direct_block[i] == 0) return;   // * Error
            read1(sockfd, clientID, inode->_direct_block[i]);
            memset(buf, 0, sizeof(buf));
            memcpy(buf, buffer[clientID], ((i == inode->_direct_count - 1 && last_block_size != 0) ? last_block_size + 1: BLOCK_SIZE));
            strcat(ans + i * BLOCK_SIZE, buf);
        }
    } else {
        for (int i = 0; i < DIRECT_LINK; ++i) {
            if (inode->_direct_block[i] == 0) return;   // * Error
            read1(sockfd, clientID, inode->_direct_block[i]);
            memset(buf, 0, sizeof(buf));
            memcpy(buf, buffer[clientID], ((i == inode->_direct_count - 1 && last_block_size != 0) ? last_block_size + 1 : BLOCK_SIZE));
            strcat(ans + i * BLOCK_SIZE, buf);
        }
        if (inode->_indirect_block == 0) return;   // * Error
        read1(sockfd, clientID, inode->_indirect_block);
        uint16_t indirect_blockid[128];
        memcpy(indirect_blockid, buffer[clientID], BLOCK_SIZE);
        for (int i = 0; i < inode->_direct_count - DIRECT_LINK; ++i) {
            read1(sockfd, clientID, indirect_blockid[i]);
            memset(buf, 0, sizeof(buf));
            memcpy(buf, buffer[clientID], ((i == inode->_direct_count - DIRECT_LINK - 1 && last_block_size != 0) ? last_block_size + 1 : BLOCK_SIZE));
            strcat(ans + (i + DIRECT_LINK) * BLOCK_SIZE, buf);
        }
    }

}

void file_write(Inode* inode, uint16_t clientID, char* src) {
    if (inode->_mode == 1) return;
    int need = (strlen(src) + BLOCK_SIZE - 1) / BLOCK_SIZE; // * ceil(strlen(src) / BLOCK_SIZE)
    char buf[BLOCK_SIZE];
    memset(buf, 0, sizeof(buf));
    char src_cp[BLOCK_SIZE * need + 1];
    char src_real[BLOCK_SIZE * need];
    memset(src_cp, 0, sizeof(src_cp));
    memset(src_real, 0, sizeof(src_real));
    memcpy(src_cp, src, strlen(src) + 1);
    memcpy(src_real, src, strlen(src));
    if (need <= inode->_direct_count) {
        if (need <= DIRECT_LINK) {
            for (int i = 0; i < need; ++i) {
                if (inode->_direct_block[i] == 0) return; // ! Never happen
                memset(buf, 0, sizeof(buf));
                memcpy(buf, src_real + i * BLOCK_SIZE, BLOCK_SIZE);
                write1(sockfd, clientID, inode->_direct_block[i], BLOCK_SIZE, buf);
            }
            for (int i = need; i < DIRECT_LINK; ++i) {
                if (inode->_direct_block[i] != 0) {
                    gc_block(inode->_direct_block[i], clientID);
                    inode->_direct_block[i] = 0;
                }
            }
            if (inode->_indirect_block != 0) {
                gc_block(inode->_indirect_block, clientID);
                inode->_indirect_block = 0;
            }
        } else {
            for (int i = 0; i < DIRECT_LINK; ++i) {
                if (inode->_direct_block[i] == 0) return; // ! Never happen
                memset(buf, 0, sizeof(buf));
                memcpy(buf, src_real + i * BLOCK_SIZE, BLOCK_SIZE);
                write1(sockfd, clientID, inode->_direct_block[i], BLOCK_SIZE, buf);
            }
            if (inode->_indirect_block == 0) return;  // ! Never happen
            uint16_t indirect_blockid[128];
            memset(indirect_blockid, 0, sizeof(indirect_blockid));
            read1(sockfd, clientID, inode->_indirect_block);
            memcpy(indirect_blockid, buffer[clientID], BLOCK_SIZE);
            for (int i = 0; i < need - DIRECT_LINK; ++i) {
                if (indirect_blockid[i] == 0) return; // ! Never happen
                memset(buf, 0, sizeof(buf));
                memcpy(buf, src_real + (i + DIRECT_LINK) * BLOCK_SIZE, BLOCK_SIZE);
                write1(sockfd, clientID, indirect_blockid[i], BLOCK_SIZE, buf);
            }
            for (int i = need - DIRECT_LINK; i < inode->_direct_count - DIRECT_LINK; ++i) {
                if (indirect_blockid[i] != 0) {
                    gc_block(indirect_blockid[i], clientID);
                    indirect_blockid[i] = 0;
                }
            }
            memset(buf, 0, sizeof(buf));
            memcpy(buf, indirect_blockid, BLOCK_SIZE);
            write1(sockfd, clientID, inode->_indirect_block, BLOCK_SIZE, buf);
        }
    } else {
        if (need <= DIRECT_LINK) {
            for (int i = 0; i < need; ++i) {
                if (inode->_direct_block[i] == 0) {
                    int blockID = alloc_block();
                    if (blockID < 0) return; // ! Could happen
                    inode->_direct_block[i] = blockID;
                }
                memset(buf, 0, sizeof(buf));
                memcpy(buf, src_real + i * BLOCK_SIZE, BLOCK_SIZE);
                write1(sockfd, clientID, inode->_direct_block[i], BLOCK_SIZE, buf);
            }
            if (inode->_indirect_block != 0) {
                gc_block(inode->_indirect_block, clientID);
                inode->_indirect_block = 0;
            }
        } else {
            for (int i = 0; i < DIRECT_LINK; ++i) {
                if (inode->_direct_block[i] == 0) {
                    int blockID = alloc_block();
                    if (blockID < 0) return; // ! Could happen
                    inode->_direct_block[i] = blockID;
                }
                memset(buf, 0, sizeof(buf));
                memcpy(buf, src_real + i * BLOCK_SIZE, BLOCK_SIZE);
                write1(sockfd, clientID, inode->_direct_block[i], BLOCK_SIZE, buf);
            }
            uint16_t indirect_blockid[128];
            memset(indirect_blockid, 0, sizeof(indirect_blockid));
            if (inode->_indirect_block == 0) {
                int blockID = alloc_block();
                if (blockID < 0) return; // ! Could happen
                inode->_indirect_block = blockID;
            } else {
                read1(sockfd, clientID, inode->_indirect_block);
                memcpy(indirect_blockid, buffer[clientID], BLOCK_SIZE);
            }
            
            for (int i = 0; i < need - DIRECT_LINK; ++i) {
                if (indirect_blockid[i] == 0) {
                    int blockID = alloc_block();
                    if (blockID < 0) return; // ! Could happen
                    indirect_blockid[i] = blockID;
                }
                memset(buf, 0, sizeof(buf));
                memcpy(buf, src_real + (i + DIRECT_LINK) * BLOCK_SIZE, BLOCK_SIZE);
                write1(sockfd, clientID, indirect_blockid[i], BLOCK_SIZE, buf);
            }
            memset(buf, 0, sizeof(buf));
            memcpy(buf, indirect_blockid, BLOCK_SIZE);
            write1(sockfd, clientID, inode->_indirect_block, BLOCK_SIZE, buf);
        }
    }
    inode->_direct_count = need;
    inode->_file_size = strlen(src);
    inode->_last_modified_timestamp = time(NULL);
    write_inode_to_disc(inode, clientID, inode->_index);
}