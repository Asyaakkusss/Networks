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
#define REQ_BUF_LEN 1024

char *port_number = NULL; 
char *root_directory = NULL; 
char *auth_token = NULL; 
unsigned short cmd_line_flags = 0;
char *REQUEST_BUFFER[REQ_BUF_LEN]; 

struct sockaddr_in sock_info;
struct sockaddr addr;
struct protoent *protoinfo;
unsigned int addrlen;
int sd, sd2;

#define MSG_POS 2
#define ERROR 1
#define QLEN 1
#define PROTOCOL "tcp"
#define BUFLEN 1024


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


void create_tcp_socket() {
    
    /* determine protocol */
    if ((protoinfo = getprotobyname (PROTOCOL)) == NULL) {
        fprintf(stderr, "cannot find protocol information for.%s\n", PROTOCOL);
        exit(1); 
    }
    /* setup endpoint info */
    memset ((char *)&sock_info,0x0,sizeof (sock_info));
    sock_info.sin_family = AF_INET;
    sock_info.sin_addr.s_addr = INADDR_ANY;
    sock_info.sin_port = htons ((u_short) atoi (port_number));

    /* allocate a socket */
    /*   would be SOCK_DGRAM for UDP */
    sd = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);
    if (sd < 0) {
        fprintf(stderr, "cannot create socket.\n");
        exit(1); 
    }

    /* bind the socket */
    if (bind (sd, (struct sockaddr *)&sock_info, sizeof(sock_info)) < 0) {
        fprintf(stderr, "cannot bind to port %s", port_number);
        exit(1); 
    }
    /* listen for incoming connections */
    if (listen (sd, QLEN) < 0) {
        fprintf(stderr, "cannot listen on port %s\n", port_number);
        exit(1); 
    }

    /* accept a connection */
    addrlen = sizeof (addr);
    sd2 = accept (sd,&addr,&addrlen);
    if (sd2 < 0) {
        fprintf(stderr, "error accepting connection.\n");
        exit(1); 
    }
    /* read information from sd2 to get the HTTP request */
    memset(REQUEST_BUFFER, 0, REQ_BUF_LEN);  
    bytes_read = read(sd2, REQUEST_BUFFER, REQ_BUF_LEN - sizeof(unsigned char)); 
    REQUEST_BUFFER[bytes_read] = '\0'; 

    /* write message to the connection */
    //if (write (sd2,argv [MSG_POS],strlen (argv [MSG_POS])) < 0) {
      //  fprintf("error writing message: %s", argv [MSG_POS]);
   // }

    /* close connections and exit */
    close (sd);
    close (sd2);
}

/*
– The request must start with a line of the form “METHOD ARGUMENT HTTP/VERSION”.
– Each line in the request must be terminated by a carriage-return and linefeed (“\r\n”).
– The HTTP request must end with a “blank” line that consists only of a carriage-return and
linefeed (“\r\n”).
– The HTTP request may contain additional, arbitrary header lines. These must be accepted, but
will be ignored by your server.
When a request arrives that does not conform to all of the above rules the web server will return
a minimal response of “HTTP/1.1 400 Malformed Request\r\n\r\n” to the client. At this point all
processing of the request is finished.
*/
void malformed_request_checked(char *req) {

    bool is_valid_request = false; 
    bool found_blank_line = false ; 

    char *first_line = strtok(req, "\r\n"); 

    if (first_line != NULL) {
        if (strncmp(first_line, "GET ", 4) == 0 || strncmp(first_line, "SHUTDOWN ", 9) == 0){
            is_valid_request = true; 
        }
            
    }
    char *carriage_check; 
    while (carriage_check = strtok(req, "\r\n") != NULL) {
        if (strlen(carriage_check) == 0) {
            found_blank_line = true; 
            break; //in case there is more which is ok 
        }
    }

    if (is_valid_request == false || found_blank_line == false) {
        fprintf(stderr, "HTTP/1.1 400 Malformed Request\r\n\r\n"); 
        exit(1); 
    }
}

int main(int argc, char *argv[]) {

    parseargs(argc,argv);

    return 0; 
}

