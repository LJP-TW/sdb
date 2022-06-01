#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <sdbcmd/exit.h>
#include <sdb.h>

int sdb_cmd_exit(int argc, char **argv)
{
    if (sdb.pid) {
        kill(sdb.pid, SIGKILL);
    }

    exit(0);
}