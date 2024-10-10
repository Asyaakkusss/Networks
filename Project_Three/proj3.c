/*
our project will be a simple HTTP server. In general, this will entail following these steps: (i) listening for
incoming TCP connections on a given port, (ii) reading an HTTP request sent by the client, (iii) sending
an HTTP response that either provides the requested file or an error message, (iv) shutting down the TCP
connection and (v) going back to step (i) to process another request.
The usage of your program—which will be called proj3 —will be run as follows:
./proj3 -p port -r document_directory -t auth_token
Specifically:
• The “-p” option must be present when running your web server and will specify the server port number
your server will listen on.
• The “-r” option must be present when running your web server and will specify the “root” directory
from which your web server will serve files. All files in this directory and all sub-directories (arbitrarily
deep) will be made available via your web server.
• The “-t” option must be present when running your web server and will specify an “authentication
token” that will be needed to terminate your web server (as explained below).
• The command line arguments may appear in any order.
• Unknown command line arguments must trigger errors.

./proj3 -p 1947 -t foobar -r ~/doc-root
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
                return; // Exit after handling error
            default:
                usage(argv[0]);
                return; // Exit after handling error
        }
    }

    validateargs(argc, argv); 
}


int main(int argc, char *argv[]) {

    parseargs(argc,argv);

   
    return 0; 
}

