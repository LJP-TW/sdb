#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sdbcmd/run.h>
#include <sdb.h>
#include <sdbcmd/cont.h>
#include <sdbcmd/start.h>

int sdb_cmd_run(int argc, char **argv)
{
    if (sdb.state != SDB_STATE_LOADED && sdb.state != SDB_STATE_RUNNING) {
        printf("** state must be LOADED or RUNNING\n");
        return 0;
    }

    if (sdb.state == SDB_STATE_RUNNING) {
        printf("** program %s is already running\n", sdb.loaded_filepath);
    } else {
        sdb_cmd_start(argc, argv);
    }

    sdb_cmd_cont(argc, argv);

    return 0;
}