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
    Elf64_Shdr shstrtab64;
    size_t size;
    int found;
    int ret;

    // TODO: fclose it properly
    file = fopen(filename, "r");
    if (!file) {
        ERROR(fopen);
    }

    size = fread(&elfhdr64, 1, sizeof(Elf64_Ehdr), file);
    if (size < sizeof(Elf64_Ehdr)) {
        goto LOAD_ERROR;
    }

    // TODO: Verify if it is a valid ELF binary

    // TODO: Support 32-bit ELF

    ret = fseek(file,
                elfhdr64.e_shoff + sizeof(Elf64_Shdr) * elfhdr64.e_shstrndx,
                SEEK_SET);
    if (ret == -1) {
        goto LOAD_ERROR;
    }

    size = fread(&shstrtab64, 1, sizeof(Elf64_Shdr), file);
    if (size < sizeof(Elf64_Shdr)) {
        goto LOAD_ERROR;
    }

    found = 0;

    for (int i = 0; i < elfhdr64.e_shnum; ++i) {
        Elf64_Shdr shdr64;
        int sh_name_offset;
        char sh_name_buf[6];

        ret = fseek(file,
                    elfhdr64.e_shoff + sizeof(Elf64_Shdr) * i,
                    SEEK_SET);
        if (ret == -1) {
            goto LOAD_ERROR;
        }

        size = fread(&shdr64, 1, sizeof(Elf64_Shdr), file);
        if (size < sizeof(Elf64_Shdr)) {
            goto LOAD_ERROR;
        }

        sh_name_offset = shstrtab64.sh_offset + shdr64.sh_name;

        ret = fseek(file, sh_name_offset, SEEK_SET);
        if (ret == -1) {
            goto LOAD_ERROR;
        }

        size = fread(sh_name_buf, 1, sizeof(sh_name_buf), file);
        if (size < sizeof(sh_name_buf)) {
            goto LOAD_ERROR;
        }

        if (!strcmp(".text", sh_name_buf)) {
            found = 1;
            sdb.stext = shdr64.sh_addr;
            sdb.etext = shdr64.sh_addr + shdr64.sh_size;
            break;
        }
    }

    if (!found) {
        goto LOAD_ERROR;
    }

    sdb.state = SDB_STATE_LOADED;
    sdb.loaded_file = file;
    sdb.loaded_filepath = strdup(filename);
    sdb.entry = elfhdr64.e_entry;

    printf("** program '%s' loaded. entry point %p\n",
        filename,
        (void *)elfhdr64.e_entry);

    return;

LOAD_ERROR:
    fprintf(stderr, "** not a valid ELF binary\n");
    fclose(file);
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