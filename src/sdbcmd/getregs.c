#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/user.h>

#include <sdbcmd/break.h>
#include <sdb.h>

static inline void do_getregs(void)
{
    struct user_regs_struct regs;

    ptrace(PTRACE_GETREGS, sdb.pid, NULL, &regs);

    printf("RAX %16llx RBX %16llx RCX %16llx RDX %16llx\n",
           regs.rax, regs.rbx, regs.rcx, regs.rdx);
    printf("R8  %16llx R9  %16llx R10 %16llx R11 %16llx\n",
           regs.r8, regs.r9, regs.r10, regs.r11);
    printf("R12 %16llx R13 %16llx R14 %16llx R15 %16llx\n",
           regs.r12, regs.r13, regs.r14, regs.r15);
    printf("RDI %16llx RSI %16llx RBP %16llx RSP %16llx\n",
           regs.rdi, regs.rsi, regs.rbp, regs.rsp);
    printf("RIP %16llx FLAGS %016llx\n",
           regs.rip, regs.eflags);
}

int sdb_cmd_getregs(int argc, char **argv)
{
    if (sdb.state != SDB_STATE_RUNNING) {
        printf("** state must be RUNNING\n");
        return 0;
    }

    do_getregs();

    return 0;
}