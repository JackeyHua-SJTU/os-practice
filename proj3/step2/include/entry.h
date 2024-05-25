#ifndef _ENTRY_H_
#define _ENTRY_H_

#include <stdint.h>

#define MAX_NAME_LEN 28

// ! sizeof(entry) is 32 bytes
// * A block can hold 8 entry
typedef struct {
    uint8_t _type;
    uint8_t _state;
    uint16_t _inode_id;
    char _name[MAX_NAME_LEN];
} Entry;

#endif