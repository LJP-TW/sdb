#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>

#include <sdbcmd/cont.h>
#include <sdb.h>

static inline void do_continue(void)
{
    sdb_breakpoint_meta *bp;
    struct user_regs_struct regs;
    uint64 data;
    int wstatus;

    sdb.running = 1;

    sdb_set_all_bp();

    ptrace(PTRACE_GETREGS, sdb.pid, NULL, &regs);
    data = ptrace(PTRACE_PEEKTEXT, sdb.pid, (void *)regs.rip, 0);

    if (data & 0xff != 0xcc) {
        goto DO_CONTINUE_END;
    }

    bp = sdb_find_bp(regs.rip, NULL);

    if (!bp) {
        goto DO_CONTINUE_END;
    }

    data = (data & 0xffffffffffffff00) | bp->data;
    ptrace(PTRACE_POKETEXT, sdb.pid, (void *)regs.rip, (void *)data);

    sdb_unset_handler();

    ptrace(PTRACE_SINGLESTEP, sdb.pid, 0, 0);

    waitpid(sdb.pid, &wstatus, 0);

    if (WIFEXITED(wstatus)) {
        sdb.state = SDB_STATE_LOADED;
        sdb.pid = 0;
        sdb.running = 0;

        printf("** child process %d terminiated normally (code %d)\n",
               sdb.pid, WEXITSTATUS(wstatus));

        return;
    }

    sdb_set_handler();

    data = (data & 0xffffffffffffff00) | 0xcc;
    ptrace(PTRACE_POKETEXT, sdb.pid, (void *)regs.rip, (void *)data);

DO_CONTINUE_END:
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