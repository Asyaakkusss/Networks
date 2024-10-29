/*
Name: Asya Akkus 
Case Network ID: aya29
File Name: proj4.c
Date Created: 10/28/2024
Description: file implements the r, i, s, t, and m options 
that process a packet trace file in different ways. 
*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define ARG_R  0x1  // Root directory (trace file)
#define ARG_I  0x2  // Trace information mode
#define ARG_S  0x4  // Size analysis mode
#define ARG_T  0x8  // TCP packet printing mode
#define ARG_M  0x10 // Traffic matrix mode

char *trace_file = NULL; 
unsigned short cmd_line_flags = 0;

void usage(char *progname) {
    fprintf(stderr, "%s -r trace_file [-i | -s | -t | -m]\n", progname);
    fprintf(stderr, "   -r    Packet trace file (must be present)\n");
    fprintf(stderr, "   -i    Trace information mode\n");
    fprintf(stderr, "   -s    Size analysis mode\n");
    fprintf(stderr, "   -t    TCP packet printing mode\n");
    fprintf(stderr, "   -m    Traffic matrix mode\n");
    exit(1);
}

void validateargs(int argc, char *argv[]) {
    // The trace file must always be present
    if ((cmd_line_flags & ARG_R) == 0 || trace_file == NULL) {
        fprintf(stderr, "error: -r option must be present\n");
        usage(argv[0]);
    }

    // Only one mode must be selected (-i, -s, -t, -m)
    int mode_count = !!(cmd_line_flags & ARG_I) + !!(cmd_line_flags & ARG_S) +
                     !!(cmd_line_flags & ARG_T) + !!(cmd_line_flags & ARG_M);
                     
    if (mode_count == 0) {
        fprintf(stderr, "error: one mode option (-i, -s, -t, -m) must be selected\n");
        usage(argv[0]);
    }
    if (mode_count > 1) {
        fprintf(stderr, "error: multiple mode options cannot be selected simultaneously\n");
        usage(argv[0]);
    }
}

void parseargs(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "r:istm")) != -1) {
        switch (opt) {
            case 'r':
                cmd_line_flags |= ARG_R;
                trace_file = optarg;
                if (trace_file == NULL) {
                    fprintf(stderr, "error: -r option must be followed by a packet trace file.\n");
                    exit(1);
                }
                break;
            case 'i':
                cmd_line_flags |= ARG_I;
                break;
            case 's':
                cmd_line_flags |= ARG_S;
                break;
            case 't':
                cmd_line_flags |= ARG_T;
                break;
            case 'm':
                cmd_line_flags |= ARG_M;
                break;
            case '?':
                usage(argv[0]);
                exit(1);
            default:
                usage(argv[0]);
                exit(1);
        }
    }

    validateargs(argc, argv);
}

int main(int argc, char *argv[]) {
    parseargs(argc, argv);
    return 0;
}
