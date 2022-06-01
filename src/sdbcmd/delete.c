#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sdbcmd/delete.h>
#include <sdb.h>

static inline void do_delete_bp(uint64 bpidx)
{
    sdb_breakpoint_meta **p, *bp;
    int n;

    p = &sdb.bplist;
    n = 0;

    while (*p) {
        if (n == bpidx) {
            bp = *p;
            *p = bp->next;
            free(bp);

            printf("** breakpoint %lld deleted\n", bpidx);
            return;
        }

        p = &(*p)->next;
        n += 1;
    }

    printf("** breakpoint %lld does not exist\n", bpidx);
}

int sdb_cmd_delete(int argc, char **argv)
{
    uint64 bpidx;
    int ret;

    if (sdb.state != SDB_STATE_RUNNING) {
        printf("** state must be RUNNING\n");
        return 0;
    }

    if (argc < 2) {
        printf("** no break-point-id is given\n");
        return 0;
    }

    ret = sscanf(argv[1], "%lld", &bpidx);

    if (ret <= 0) {
        return 0;
    }

    do_delete_bp(bpidx);

    return 0;
}