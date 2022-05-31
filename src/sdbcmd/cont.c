#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <sdbcmd/cont.h>
#include <sdb.h>

static inline void do_continue(void)
{
    sdb.running = 1;

    ptrace(PTRACE_CONT, sdb.pid, NULL, NULL);
}

int sdb_cmd_cont(int argc, char **argv)
{
    if (sdb.state != SDB_STATE_RUNNING) {
        printf("** state must be RUNNING\n");
        return 0;
    }

    do_continue();

    return 0;
}