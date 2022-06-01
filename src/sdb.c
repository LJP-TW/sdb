#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>

#include <sdb.h>
#include <sdbcmd.h>
#include <argv.h>

#define MAX_CMD_ARGV 8

sdb_meta sdb;

#define SDB_CMD_DEFINE1(CMDNAME) \
    do {                                                              \
        sdb_command_meta *meta = malloc(sizeof(sdb_command_meta));    \
        meta->next = NULL;                                            \
        meta->name = strdup(#CMDNAME);                                \
        meta->shortname = NULL;                                       \
        meta->func = sdb_cmd_##CMDNAME;                               \
                                                                      \
        *list = meta;                                                 \
        list = &meta->next;                                           \
    } while(0)

#define SDB_CMD_DEFINE2(CMDNAME, SHORTNAME) \
    do {                                                              \
        sdb_command_meta *meta = malloc(sizeof(sdb_command_meta));    \
        meta->next = NULL;                                            \
        meta->name = strdup(#CMDNAME);                                \
        meta->shortname = strdup(#SHORTNAME);                         \
        meta->func = sdb_cmd_##CMDNAME;                               \
                                                                      \
        *list = meta;                                                 \
        list = &meta->next;                                           \
    } while(0)

typedef int (*sdb_command_funcp)(int argc, char **argv);

typedef struct _sdb_command_meta {
    struct _sdb_command_meta *next;
    char *name;
    char *shortname;
    sdb_command_funcp func;
} sdb_command_meta;

sdb_command_meta *sdb_command_meta_list;

static void sdb_resume_from_bp(void);
static void sdb_handler(int sig, siginfo_t *info, void *ucontext);
static void sdb_init(void);
static void sdb_loop(void);
static void sdb_prompt(void);
static int sdb_read_cmdline(char *buf);
static int sdb_parse_cmdline(char *cmdline, char **argv);
static sdb_command_meta *sdb_get_cmd_meta(char *cmdname);
static void sdb_dispatch_cmd(sdb_command_meta *cmd, int argc, char **argv);

int main(int argc, char **argv)
{   
    parse_args(argc, argv);
    
    sdb_init();

    sdb_loop();
}

void sdb_set_handler(void)
{
    struct sigaction action;

    sigemptyset(&action.sa_mask);
    action.sa_sigaction = sdb_handler;
    action.sa_flags = SA_SIGINFO;

    sigaction(SIGCHLD, &action, NULL);
}

void sdb_unset_handler(void)
{
    struct sigaction action;

    sigemptyset(&action.sa_mask);
    action.sa_handler = SIG_DFL;
    action.sa_flags = 0;

    sigaction(SIGCHLD, &action, NULL);
}

void sdb_set_new_bp(uint64 addr)
{
    sdb_breakpoint_meta *bp, **p;
    uint64 data;
    int idx;

    bp = sdb_find_bp(addr, &idx);

    if (bp) {
        printf("** the breakpoint is already exists. (breakpoint %d)\n", idx);
        return;
    }

    errno = 0;

    // TODO: Check the range of the text segment
    data = ptrace(PTRACE_PEEKTEXT, sdb.pid, (void *)addr, 0);

    if (errno) {
        errno = 0;

        printf("** the address is out of the range of the text segment\n");
        return;
    }

    bp = malloc(sizeof(sdb_breakpoint_meta));
    bp->next = NULL;
    bp->addr = addr;
    bp->data = data & 0xff;

    p = &sdb.bplist;
    while (*p) {
        p = &(*p)->next;
    }
    *p = bp;
}

void sdb_set_all_bp(void)
{
    sdb_breakpoint_meta **p;
    
    p = &sdb.bplist;

    while (*p) {
        uint64 newdata, olddata, addr;

        addr = (*p)->addr;
        olddata = ptrace(PTRACE_PEEKTEXT, sdb.pid, (void *)addr, 0);
        newdata = (olddata & 0xffffffffffffff00) | 0xCC;
        ptrace(PTRACE_POKETEXT, sdb.pid, (void *)addr, (void *)newdata);

        p = &(*p)->next;
    }
}

void sdb_resume_all_bp(void)
{
    sdb_breakpoint_meta **p;
    
    p = &sdb.bplist;

    while (*p) {
        uint64 newdata, olddata, addr;

        addr = (*p)->addr;
        olddata = ptrace(PTRACE_PEEKTEXT, sdb.pid, (void *)addr, 0);
        newdata = (olddata & 0xffffffffffffff00) | (*p)->data;
        ptrace(PTRACE_POKETEXT, sdb.pid, (void *)addr, (void *)newdata);

        p = &(*p)->next;
    }
}

int sdb_show_disasm(uint64 addr)
{
    cs_insn *insn;
    size_t count;
    char code[0x10];
    int i;

    if (addr < sdb.stext || sdb.etext <= addr) {
        printf("** the address is out of the range of the text segment\n");
        return -1;
    }

    *(uint64 *)(&code[0]) = ptrace(PTRACE_PEEKTEXT, sdb.pid,
                                   (void *)addr, 0);
    *(uint64 *)(&code[8]) = ptrace(PTRACE_PEEKTEXT, sdb.pid,
                                   (void *)addr + 8, 0);

    count = cs_disasm(sdb.capstone, code, sizeof(code)-1, addr, 0, &insn);
    if (count <= 0) {
        printf("** failed to disassemble given code (0x%llx)\n", addr);
        return -1;
    }

    printf("\t%lx: ", insn[0].address);
    for (i = 0; i < insn[0].size; ++i) {
        printf("%02x ", insn[0].bytes[i]);
    }
    for (; i < 16; ++i) {
        printf("   ");
    }    
    printf("\t%s\t%s\n", insn[0].mnemonic, insn[0].op_str);
    
    cs_free(insn, count);

    return insn[0].size;
}

sdb_breakpoint_meta *sdb_find_bp(uint64 addr, int *idx)
{
    sdb_breakpoint_meta **p;
    int n;

    p = &sdb.bplist;
    n = 0;

    while (*p) {
        if ((*p)->addr == addr) {
            if (idx) {
                *idx = n;
            }

            return *p;
        }

        p = &(*p)->next;
        n += 1;
    }

    return NULL;
}

static void sdb_resume_from_bp(void)
{
    sdb_breakpoint_meta *bp;
    struct user_regs_struct regs;

    ptrace(PTRACE_GETREGS, sdb.pid, NULL, &regs);

    bp = sdb_find_bp(regs.rip - 1, NULL);

    if (!bp) {
        return;
    }

    regs.rip -= 1;
    ptrace(PTRACE_SETREGS, sdb.pid, NULL, &regs);

    sdb_resume_all_bp();

    printf("** breakpoint @");
    sdb_show_disasm(regs.rip);
}

static void sdb_handler(int sig, siginfo_t *info, void *ucontext)
{
    if (info->si_code == CLD_EXITED) {
        waitpid(sdb.pid, NULL, 0);

        printf("** child process %d terminiated normally (code %d)\n",
               sdb.pid, info->si_status);

        sdb.state = SDB_STATE_LOADED;
        sdb.pid = 0;
        sdb.running = 0;

        sdb_unset_handler();
    } else if (info->si_code == CLD_TRAPPED) {
        sdb_resume_from_bp();

        sdb.running = 0;
    } else {
        printf("** TODO: Handle this situation\n");
    }
}

static void sdb_init(void)
{
    sdb_command_meta **list;

    // Initialize sdb
    sdb.state = SDB_STATE_EMPTY;

    if (cs_open(CS_ARCH_X86, CS_MODE_64, &sdb.capstone) != CS_ERR_OK) {
        exit(1);
    }

    // Define commands
    list = &sdb_command_meta_list;

    SDB_CMD_DEFINE2(break, b);
    SDB_CMD_DEFINE2(cont, c);
    SDB_CMD_DEFINE1(delete);
    SDB_CMD_DEFINE2(disasm, d);
    SDB_CMD_DEFINE2(dump, x);
    SDB_CMD_DEFINE2(get, g);
    SDB_CMD_DEFINE1(getregs);
    SDB_CMD_DEFINE2(help, h);
    SDB_CMD_DEFINE2(list, l);
    SDB_CMD_DEFINE1(load);
    SDB_CMD_DEFINE2(run, r);
    SDB_CMD_DEFINE2(set, s);
    SDB_CMD_DEFINE1(si);
    SDB_CMD_DEFINE1(start);
    SDB_CMD_DEFINE2(vmmap, m);
}

static void sdb_loop(void)
{
    while (1) {
        sdb_command_meta *cmd;
        int ret;
        int argc;
        char *argv[MAX_CMD_ARGV];
        char buf[256];

        ret = 0;

        while (sdb.running) {}

        while (!ret) {
            sdb_prompt();
            ret = sdb_read_cmdline(buf);
        }

        argc = sdb_parse_cmdline(buf, argv);

        cmd = sdb_get_cmd_meta(argv[0]);
        
        if (!cmd) {
            continue;
        }

        sdb_dispatch_cmd(cmd, argc, argv);
    }
}

static void sdb_prompt(void)
{
    printf("sdb> ");
}

static int sdb_read_cmdline(char *buf)
{
    int ret = 0;
    char c;
    
    while (!ret) {
        ret = scanf("%255[^\n]%*c", buf);

        if (ret < 0) {
            // TODO: Handle scanf error
            exit(1);
        }

        if (!ret) {
            scanf("%c", &c);

            if (c == '\n') {
                return 0;
            }
        }
    }

    return ret;
}

static int sdb_parse_cmdline(char *cmdline, char **argv)
{
    int argc = 1;
    char *p = cmdline;

    argv[0] = cmdline;

    while (*p) {
        if (*p == ' ') {
            *p = '\0';

            if (argc >= MAX_CMD_ARGV) {
                // TODO: Handle this situation
                break;
            }

            argv[argc] = p + 1;
            argc += 1;
        }

        p++;
    }

    return argc;
}

static sdb_command_meta *sdb_get_cmd_meta(char *cmdname)
{
    sdb_command_meta *cmd;

    for (cmd = sdb_command_meta_list; cmd; cmd = cmd->next) {
        if (cmd->shortname && !strcmp(cmd->shortname, cmdname)) {
            break;
        }

        if (!strcmp(cmd->name, cmdname)) {
            break;
        }
    }

    if (!cmd) {
        // TODO: Command not found
        return NULL;
    }

    return cmd;
}

static void sdb_dispatch_cmd(sdb_command_meta *cmd, int argc, char **argv)
{
    cmd->func(argc, argv);
}