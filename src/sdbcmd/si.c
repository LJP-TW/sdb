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
    uint64 data;
    int wstatus;

    sdb.running = 1;

    sdb_set_all_bp();

    ptrace(PTRACE_GETREGS, sdb.pid, NULL, &regs);
    data = ptrace(PTRACE_PEEKTEXT, sdb.pid, (void *)regs.rip, 0);

    if (data & 0xff != 0xcc) {
        goto DO_STEP_INS_END;
    }

    bp = sdb_find_bp(regs.rip, NULL);

    if (!bp) {
        goto DO_STEP_INS_END;
    }

    // Hit breakpoint again

    sdb_resume_all_bp();

    sdb_unset_handler();

    ptrace(PTRACE_SINGLESTEP, sdb.pid, 0, 0);

    waitpid(sdb.pid, &wstatus, 0);

    sdb.running = 0;

    if (WIFEXITED(wstatus)) {
        printf("** child process %d terminiated normally (code %d)\n",
               sdb.pid, WEXITSTATUS(wstatus));

        sdb.state = SDB_STATE_LOADED;
        sdb.pid = 0;
        return;
    }

    sdb_set_handler();

    ptrace(PTRACE_GETREGS, sdb.pid, NULL, &regs);

    printf("** breakpoint @");
    sdb_show_disasm(regs.rip);

    return;

DO_STEP_INS_END:
    ptrace(PTRACE_SINGLESTEP, sdb.pid, 0, 0);
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