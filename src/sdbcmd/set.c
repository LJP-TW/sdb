#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/user.h>

#include <sdbcmd/set.h>
#include <sdb.h>

#define SET_REGISTER(register, val) \
    regs.register = val

#define MAYBE_SET_REGISTER(register)                             \
    if (!strcasecmp(reg, #register)) {                           \
        SET_REGISTER(register, val);                             \
        break;                                                   \
    }

#define MAYBE_SET_REGISTER_ALIAS(register, elem)                 \
    if (!strcasecmp(reg, #register)) {                           \
        SET_REGISTER(elem, val);                                 \
        break;                                                   \
    }

static inline void do_set_reg(char *reg, uint64 val)
{
    struct user_regs_struct regs;

    ptrace(PTRACE_GETREGS, sdb.pid, NULL, &regs);

    do {
        // TODO: Support multi-length registers (e.g. eax, ax, al, ah ...)

        MAYBE_SET_REGISTER(rax);
        MAYBE_SET_REGISTER(rbx);
        MAYBE_SET_REGISTER(rcx);
        MAYBE_SET_REGISTER(rdx);
        MAYBE_SET_REGISTER(rdi);
        MAYBE_SET_REGISTER(rsi);
        MAYBE_SET_REGISTER(rbp);
        MAYBE_SET_REGISTER(rsp);
        MAYBE_SET_REGISTER(r8);
        MAYBE_SET_REGISTER(r9);
        MAYBE_SET_REGISTER(r10);
        MAYBE_SET_REGISTER(r11);
        MAYBE_SET_REGISTER(r12);
        MAYBE_SET_REGISTER(r13);
        MAYBE_SET_REGISTER(r14);
        MAYBE_SET_REGISTER(r15);
        MAYBE_SET_REGISTER(rip);
        MAYBE_SET_REGISTER_ALIAS(flags, eflags);
    } while (0);

    ptrace(PTRACE_SETREGS, sdb.pid, NULL, &regs);
}

int sdb_cmd_set(int argc, char **argv)
{
    uint64 val;
    int ret;

    if (sdb.state != SDB_STATE_RUNNING) {
        printf("** state must be RUNNING\n");
        return 0;
    }

    if (argc < 3) {
        printf("** Not enough input arguments\n");
        return 0;
    }

    ret = sscanf(argv[2], "%llx", &val);

    if (ret <= 0) {
        return 0;
    }

    do_set_reg(argv[1], val);

    return 0;
}