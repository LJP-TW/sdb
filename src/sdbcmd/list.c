#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sdbcmd/list.h>
#include <sdb.h>

static inline void do_list_bps(void)
{
    sdb_breakpoint_meta **p;
    int n;

    p = &sdb.bplist;
    n = 0;

    while (*p) {
        printf("  %d: %llx\n", n, (*p)->addr);

        p = &(*p)->next;
        n += 1;
    }
}

int sdb_cmd_list(int argc, char **argv)
{
    do_list_bps();
}