#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <argv.h>
#include <sdb.h>

sdb_args_meta sdb_args;

void parse_args(int argc, char **argv)
{
    int c;

    while ((c = getopt(argc, argv, "s:")) != -1) {
        switch (c) {
        case 's':
            if (sdb_args.script)
                goto PARSE_ARGS_ERROR;
            sdb_args.script = strdup(optarg);
            break;
        default:
            goto PARSE_ARGS_ERROR;
        }
    }

    if (optind < argc) {
        sdb_args.program = strdup(argv[optind]);
    }

    return;

PARSE_ARGS_ERROR:
    if (argc)
        printf("usage: %s [-s script] [program]\n", argv[0]);
    exit(1);
}