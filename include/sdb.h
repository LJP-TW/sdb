#ifndef _SDB_H
#define _SDB_H

#include <stdio.h>

#include <capstone/capstone.h>

typedef unsigned char uint8;
typedef unsigned long long int uint64;

#define SDB_STATE_EMPTY        0x01
#define SDB_STATE_LOADED       0x02
#define SDB_STATE_RUNNING      0x04
#define SDB_STATE_ANY (SDB_STATE_EMPTY | SDB_STATE_LOADED | SDB_STATE_RUNNING)

typedef struct _sdb_breakpoint_meta {
    struct _sdb_breakpoint_meta *next;
    uint64 addr;
    uint8 data;
} sdb_breakpoint_meta;

typedef struct {
    int state;
    FILE *loaded_file;
    char *loaded_filepath;
    uint64 entry;
    uint64 stext;
    uint64 etext;
    pid_t pid;
    // Is debugee running
    int running;
    sdb_breakpoint_meta *bplist;
    csh capstone;
} sdb_meta;

extern sdb_meta sdb;

void sdb_set_handler(void);
void sdb_unset_handler(void);

sdb_breakpoint_meta *sdb_find_bp(uint64 addr, int *idx);
void sdb_set_new_bp(uint64 addr);

void sdb_set_all_bp(void);
void sdb_resume_all_bp(void);

int sdb_show_disasm(uint64 addr);

#endif /* _SDB_H */