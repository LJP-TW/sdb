#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <elf.h>

#include <sdbcmd/load.h>
#include <sdb.h>
#include <utils.h>

static inline void do_load(char *filename)
{
    FILE *file;
    Elf64_Ehdr elfhdr64;
    size_t size;

    file = fopen(filename, "r");
    if (!file) {
        ERROR(fopen);
    }

    size = fread(&elfhdr64, 1, sizeof(Elf64_Ehdr), file);
    if (size < sizeof(Elf64_Ehdr)) {
        fprintf(stderr, "** not a valid ELF binary\n");
        return;
    }

    // TODO: Verify if it is a valid ELF binary

    // TODO: Support 32-bit ELF

    sdb.state = SDB_STATE_LOADED;
    sdb.loaded_file = file;
    sdb.loaded_filepath = strdup(filename);
    sdb.entry = elfhdr64.e_entry;

    printf("** program '%s' loaded. entry point %p\n",
        filename,
        (void *)elfhdr64.e_entry);
}

int sdb_cmd_load(int argc, char **argv)
{
    if (sdb.state != SDB_STATE_EMPTY) {
        printf("** state must be NOT LOADED\n");
        return 0;
    }

    if (argc < 2) {
        printf("** no program path is given\n");
        return 0;
    }

    do_load(argv[1]);

    return 0;
}
