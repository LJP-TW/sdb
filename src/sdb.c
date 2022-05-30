#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sdb.h>
#include <sdbcmd.h>
#include <argv.h>

#define MAX_CMD_ARGV 8

sdb_meta sdb;

#define SDB_CMD_DEFINE(CMDNAME) \
    do {                                                              \
        sdb_cmd_meta *meta = malloc(sizeof(sdb_cmd_meta));            \
        meta->next = NULL;                                            \
        meta->name = strdup(#CMDNAME);                                \
        meta->func = sdb_cmd_##CMDNAME;                               \
                                                                      \
        *list = meta;                                                 \
        list = &meta->next;                                           \
    } while(0)

typedef int (*sdb_cmd_funcp)(int argc, char **argv);

typedef struct _sdb_cmd_meta {
    struct _sdb_cmd_meta *next;
    char *name;
    sdb_cmd_funcp func;
} sdb_cmd_meta;

sdb_cmd_meta *sdb_cmd_list;

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

static void sdb_init(void)
{
    sdb.state = SDB_STATE_EMPTY;

    sdb_cmd_meta **list = &sdb_cmd_list;

    SDB_CMD_DEFINE(help);
    SDB_CMD_DEFINE(load);
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