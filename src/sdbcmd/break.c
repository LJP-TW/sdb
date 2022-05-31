#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sdbcmd/break.h>
#include <sdb.h>

static inline void do_break(uint64 addr)
{
    sdb_set_new_bp(addr);
}

int sdb_cmd_break(int argc, char **argv)
{
    uint64 addr;
    int ret;

    if (sdb.state != SDB_STATE_RUNNING) {
        printf("** state must be RUNNING\n");
        return 0;
    }

    if (argc < 2) {
        printf("** no address is given\n");
        return 0;
    }

    ret = sscanf(argv[1], "%llx", &addr);

    if (ret <= 0) {
        return 0;
    }

    do_break(addr);

    return 0;
}