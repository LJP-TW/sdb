#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>

#include <sdbcmd/dump.h>
#include <sdb.h>

static inline void print_ascii(uint64 data)
{
    for (int i = 0; i < 8; ++i) {
        uint8 c = (data >> (i * 8)) & 0xff;

        if (c < 0x20 || 0x7e < c) {
            c = '.';
        }

        printf("%c", c);
    }
}

static inline void print_hex(uint64 data)
{
    for (int i = 0; i < 8; ++i) {
        uint8 c = (data >> (i * 8)) & 0xff;
        printf("%02x ", c);
    }
}

/* 
 * The output should include the machine code 0xcc if there is a break point.
 *
 * TODO: Report error when dumping address that is not in address space
 */
static inline void do_dump(uint64 addr)
{
    uint64 data1, data2;
    
    sdb_set_all_bp();

    for (int i = 0; i < 5; ++i) {
        data1 = ptrace(PTRACE_PEEKTEXT, sdb.pid, (void *)addr, 0);
        data2 = ptrace(PTRACE_PEEKTEXT, sdb.pid, (void *)addr + 8, 0);

        printf("\t%llx: ", addr);
        print_hex(data1);
        print_hex(data2);
        printf("|");
        print_ascii(data1);
        print_ascii(data2);
        printf("|\n");
        
        addr += 0x10;
    }

    sdb_resume_all_bp();
}

int sdb_cmd_dump(int argc, char **argv)
{
    uint64 addr;
    int ret;

    if (sdb.state != SDB_STATE_RUNNING) {
        printf("** state must be RUNNING\n");
        return 0;
    }

    if (argc < 2) {
        printf("** no addr is given\n");
        return 0;
    }

    ret = sscanf(argv[1], "%llx", &addr);

    if (ret <= 0) {
        return 0;
    }

    do_dump(addr);

    return 0;
}