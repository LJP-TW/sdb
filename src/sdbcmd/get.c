#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/user.h>

#include <sdbcmd/get.h>
#include <sdb.h>

#define SHOW_REGISTER(register, value) \
    printf(#register " = %1$llu (0x%1$llx)\n", value)

#define MAYBE_SHOW_REGISTER(register)                            \
    if (!strcasecmp(reg, #register)) {                           \
        SHOW_REGISTER(register, regs.register);                  \
        break;                                                   \
    }

#define MAYBE_SHOW_REGISTER_ALIAS(register, elem)                \
    if (!strcasecmp(reg, #register)) {                           \
        SHOW_REGISTER(register, regs.elem);                      \
        break;                                                   \
    }

static inline void get_reg(char *reg)
{
    struct user_regs_struct regs;

    ptrace(PTRACE_GETREGS, sdb.pid, NULL, &regs);

    do {
        // TODO: Support multi-length registers (e.g. eax, ax, al, ah ...)

        MAYBE_SHOW_REGISTER(rax);
        MAYBE_SHOW_REGISTER(rbx);
        MAYBE_SHOW_REGISTER(rcx);
        MAYBE_SHOW_REGISTER(rdx);
        MAYBE_SHOW_REGISTER(rdi);
        MAYBE_SHOW_REGISTER(rsi);
        MAYBE_SHOW_REGISTER(rbp);
        MAYBE_SHOW_REGISTER(rsp);
        MAYBE_SHOW_REGISTER(r8);
        MAYBE_SHOW_REGISTER(r9);
        MAYBE_SHOW_REGISTER(r10);
        MAYBE_SHOW_REGISTER(r11);
        MAYBE_SHOW_REGISTER(r12);
        MAYBE_SHOW_REGISTER(r13);
        MAYBE_SHOW_REGISTER(r14);
        MAYBE_SHOW_REGISTER(r15);
        MAYBE_SHOW_REGISTER(rip);
        MAYBE_SHOW_REGISTER_ALIAS(flags, eflags);
    } while (0);
}

int sdb_cmd_get(int argc, char **argv)
{
    if (sdb.state != SDB_STATE_RUNNING) {
        printf("** state must be RUNNING\n");
        return 0;
    }

    if (argc < 2) {
        printf("** no register is given\n");
        return 0;
    }

    get_reg(argv[1]);

    return 0;
}