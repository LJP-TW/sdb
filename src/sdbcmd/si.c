#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>

#include <sdbcmd/cont.h>
#include <sdb.h>

static inline void do_step_ins(void)
{
    sdb_breakpoint_meta *bp;
    struct user_regs_struct regs;
    int wstatus;

    sdb_unset_handler();

    ptrace(PTRACE_SINGLESTEP, sdb.pid, 0, 0);

    waitpid(sdb.pid, &wstatus, 0);

    if (WIFEXITED(wstatus)) {
        printf("** child process %d terminiated normally (code %d)\n",
               sdb.pid, WEXITSTATUS(wstatus));

        sdb.state = SDB_STATE_LOADED;
        sdb.pid = 0;
        return;
    }

    ptrace(PTRACE_GETREGS, sdb.pid, NULL, &regs);

    bp = sdb_find_bp(regs.rip, NULL);

    if (bp) {
        printf("** breakpoint @");
        sdb_show_disasm(regs.rip);
    }

    sdb_set_handler();
}

int sdb_cmd_si(int argc, char **argv)
{
    if (sdb.state != SDB_STATE_RUNNING) {
        printf("** state must be RUNNING\n");
        return 0;
    }

    do_step_ins();

    return 0;
}