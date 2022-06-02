#ifndef _ARGV_H
#define _ARGV_H

typedef struct {
    char *script;
    char *program;
} sdb_args_meta;

extern sdb_args_meta sdb_args;

void parse_args(int argc, char **argv);

#endif /* _ARGV_H */