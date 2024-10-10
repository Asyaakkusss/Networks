/*
Name: Asya Akkus 
Case Network ID: aya29
File Name: Makefile 
Date Created: 9/24/2024
Description: This is the main location of the code for project 3. It implements the P, T, and R options.
*/

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

#define ARG_P  0x1  //1
#define ARG_T  0x2  //2
#define ARG_R  0x4  //4

char *port_number = NULL; 
char *root_directory = NULL; 
char *auth_token = NULL; 
unsigned short cmd_line_flags = 0;


void usage (char *progname) {
    fprintf (stderr,"%s -p port -t authentication -r root\n", progname);
    fprintf (stderr, "   -p    Port number\n"); 
    fprintf (stderr, "   -t    Authentication Token\n"); 
    fprintf (stderr, "   -r    Root Directory\n"); 
    exit (1);
}

void validateargs (int argc, char *argv []) {

    // The filename must be present
    if ((cmd_line_flags & ARG_P) == 0 || port_number == NULL) {
        fprintf(stderr, "error: -p option must be present\n");
        usage(argv[0]);
    }

    // The URL must be present
    if ((cmd_line_flags & ARG_T) == 0 || auth_token == NULL) {
        fprintf(stderr, "error: -t option must be present\n"); 
        usage(argv[0]);
    }

    if ((cmd_line_flags & ARG_R) == 0 || root_directory == NULL) {
        fprintf(stderr, "error: -r option must be present\n"); 
        usage(argv[0]);
    }

}

void parseargs (int argc, char *argv []) {
    int opt;

    while ((opt = getopt (argc, argv, "p:t:r:")) != -1) {
        switch (opt) {
            case 'p':
              cmd_line_flags |= ARG_P;
              port_number = optarg; 
              if (port_number == NULL) {
                fprintf(stderr, "error: -p option must be followed by a port number.\n"); 
                exit(1); 
              }
              break;
            case 't':
              cmd_line_flags |= ARG_T;
              auth_token = optarg; 
              break;
            case 'r':
              cmd_line_flags |= ARG_R;
              root_directory = optarg; 
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

    parseargs(argc,argv);

    return 0; 
}

