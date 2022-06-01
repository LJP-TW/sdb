#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sdbcmd/disasm.h>
#include <sdb.h>

static inline void do_disasm(uint64 addr)
{
    for (int i = 0; i < 10; ++i) {
        int insn_size;

        insn_size = sdb_show_disasm(addr);

        if (insn_size == -1) {
            break;
        }

        addr += insn_size;
    }
}

int sdb_cmd_disasm(int argc, char **argv)
{
    uint64 addr;
    int ret;

    if (sdb.state != SDB_STATE_RUNNING) {
        printf("** state must be RUNNING\n");
        return 0;
    }

    if (argc < 2) {
        printf("** no addr is given.\n");
        return 0;
    }

    ret = sscanf(argv[1], "%llx", &addr);

    if (ret <= 0) {
        return 0;
    }

    do_disasm(addr);

    return 0;
}