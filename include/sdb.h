#ifndef _SDB_H
#define _SDB_H

#include <stdio.h>

typedef unsigned long long int uint64;

#define SDB_STATE_EMPTY        0x01
#define SDB_STATE_LOADED       0x02
#define SDB_STATE_RUNNING      0x04
#define SDB_STATE_ANY (SDB_STATE_EMPTY | SDB_STATE_LOADED | SDB_STATE_RUNNING)

typedef struct {
    int state;
    FILE *loaded_file;
    char *loaded_filepath;
    uint64 entry;
} sdb_meta;

extern sdb_meta sdb;

#endif /* _SDB_H */