#ifndef _USER_H_
#define _USER_H_

#define MAX_USER_ID 16
#define MAX_USER_KEY 16

typedef struct {
    char _user_id[MAX_USER_ID];
    char _user_key[MAX_USER_KEY];
} User;

#endif