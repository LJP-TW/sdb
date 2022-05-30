#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <sdbcmd/start.h>
#include <sdb.h>
#include <utils.h>

static inline void run_debugee(void)
{
    char *p, *filename;
    int ret;

    filename = p = sdb.loaded_filepath;
    while (*p) {
        if (*p == '/') {
            filename = p + 1;
        }
        p++;
    }

    ret = ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    if (ret == -1) {
        ERROR(ptrace);
    }

    // TODO: Send arguments properly
    execl(sdb.loaded_filepath, filename, NULL);

    // Should never return
    exit(-1);
}

static inline void do_start(void)
{
    pid_t pid;
    int wstatus;

    pid = fork();

    if (!pid) {
        run_debugee();
    } else if (pid > 0) {
        printf("** pid %d\n", pid);

        wait(&wstatus);

        if (WIFEXITED(wstatus)) {
            // TODO: Handle this situation (execl failed)
            printf("** child process %d terminiated (execl failed)\n", pid);
            return;
        }

        sdb.state = SDB_STATE_RUNNING;
        sdb.pid = pid;
    } else {
        ERROR(fork);
    }
}

int sdb_cmd_start(int argc, char **argv)
{
    if (sdb.state != SDB_STATE_LOADED) {
        printf("** state must be LOADED\n");
        return 0;
    }

    do_start();

    return 0;
}