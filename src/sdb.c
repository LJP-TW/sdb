#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#include <sdb.h>
#include <sdbcmd.h>
#include <argv.h>

#define MAX_CMD_ARGV 8

sdb_meta sdb;

#define SDB_CMD_DEFINE1(CMDNAME) \
    do {                                                              \
        sdb_cmd_meta *meta = malloc(sizeof(sdb_cmd_meta));            \
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
        sdb_cmd_meta *meta = malloc(sizeof(sdb_cmd_meta));            \
        meta->next = NULL;                                            \
        meta->name = strdup(#CMDNAME);                                \
        meta->shortname = strdup(#SHORTNAME);                         \
        meta->func = sdb_cmd_##CMDNAME;                               \
                                                                      \
        *list = meta;                                                 \
        list = &meta->next;                                           \
    } while(0)

typedef int (*sdb_cmd_funcp)(int argc, char **argv);

typedef struct _sdb_cmd_meta {
    struct _sdb_cmd_meta *next;
    char *name;
    char *shortname;
    sdb_cmd_funcp func;
} sdb_cmd_meta;

sdb_cmd_meta *sdb_cmd_list;

static void sdb_handler(int sig, siginfo_t *info, void *ucontext);
static void sdb_init(void);
static void sdb_loop(void);
static void sdb_prompt(void);
static int sdb_read_cmdline(char *buf);
static int sdb_parse_cmdline(char *cmdline, char **argv);
static sdb_cmd_meta *sdb_get_cmd_meta(char *cmdname);
static void sdb_dispatch_cmd(sdb_cmd_meta *cmd, int argc, char **argv);

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

static void sdb_handler(int sig, siginfo_t *info, void *ucontext)
{
    if (info->si_code == CLD_EXITED) {
        waitpid(sdb.pid, NULL, 0);

        sdb.state = SDB_STATE_LOADED;
        sdb.pid = 0;
        sdb.running = 0;

        printf("** child process %d terminiated normally (code %d)\n",
               sdb.pid, info->si_status);
    } else if (info->si_code == CLD_TRAPPED) {
        // TODO
        sdb.running = 0;

        printf("** Trapped\n");
    } else {
        printf("** TODO: Handle this situation\n");
    }
}

static void sdb_init(void)
{
    sdb_cmd_meta **list;

    // Initialize sdb
    sdb.state = SDB_STATE_EMPTY;

    // Define commands
    list = &sdb_cmd_list;

    SDB_CMD_DEFINE2(cont, c);
    SDB_CMD_DEFINE2(get, g);
    SDB_CMD_DEFINE2(help, h);
    SDB_CMD_DEFINE1(load);
    SDB_CMD_DEFINE1(start);
    SDB_CMD_DEFINE2(vmmap, m);
}

static void sdb_loop(void)
{
    while (1) {
        sdb_cmd_meta *cmd;
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

static sdb_cmd_meta *sdb_get_cmd_meta(char *cmdname)
{
    sdb_cmd_meta *cmd;

    for (cmd = sdb_cmd_list; cmd; cmd = cmd->next) {
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

static void sdb_dispatch_cmd(sdb_cmd_meta *cmd, int argc, char **argv)
{
    cmd->func(argc, argv);
}