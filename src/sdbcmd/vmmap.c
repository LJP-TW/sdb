#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sdbcmd/vmmap.h>
#include <sdb.h>
#include <utils.h>

static inline void do_vmmap(void)
{
    FILE *mapsfile;
    char *retp;
    int ret;
    uint64 saddr, eaddr, offset;
    char perm[16];
    char buf[256];
    char pathname[256];
    char maps[256];

    sprintf(maps, "/proc/%d/maps", sdb.pid);

    mapsfile = fopen(maps, "r");
    if (!mapsfile) {
        ERROR(fopen);
    }

    while (1) {
        retp = fgets(buf, 256, mapsfile);

        if (!retp) {
            break;
        }

        ret = sscanf(buf, "%llx-%llx %[^ ] %llx %*[^ ] %*[^ ] %s",
                     &saddr, &eaddr, perm, &offset, pathname);

        perm[3] = '\0';

        if (ret == 4) {
            printf("%016llx-%016llx %s %6llx\n",
                   saddr, eaddr, perm, offset);
        } else {
            printf("%016llx-%016llx %s %6llx\t%s\n",
                   saddr, eaddr, perm, offset, pathname);
        }
    }

    fclose(mapsfile);
}

int sdb_cmd_vmmap(int argc, char **argv)
{
    if (sdb.state != SDB_STATE_RUNNING) {
        printf("** state must be RUNNING\n");
        return 0;
    }

    do_vmmap();

    return 0;
}