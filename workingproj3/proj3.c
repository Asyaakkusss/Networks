/*
Name: Asya Akkus 
Case Network ID: aya29
File Name: Makefile 
Date Created: 9/24/2024
Description: This is the main location of the code for project 3. It implements the P, T, and R options.
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

#define ARG_P  0x1  //1
#define ARG_T  0x2  //2
#define ARG_R  0x4  //4
#define REQ_BUF_LEN 1024

char *port_number = NULL; 
char *root_directory = NULL; 
char *auth_token = NULL; 
unsigned short cmd_line_flags = 0;
char REQUEST_BUFFER[REQ_BUF_LEN]; 

struct sockaddr_in sock_info;
struct sockaddr addr;
struct protoent *protoinfo;
unsigned int addrlen;
int sd, sd2;
int BYTES_READ; 

#define MSG_POS 2
#define ERROR 1
#define QLEN 1
#define PROTOCOL "tcp"
#define BUFLEN 1024
#define GET_LEN 4
#define SHUTDOWN_LEN 9 
#define ERROR_LENS 35
#define PROT_NO_IMPLEMENT_LEN 41
#define PROTOCOL_LEN 5
#define FILE_NOT_FOUND_LEN 31
#define FILENAME_BAD 33
#define SERVER_ERR_LEN 37
#define BAD_REQ_LEN 29
#define OK_LEN 19
#define MALFORMED_FILE_LEN 50



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

void malformed_request_checker(int socket) {

    bool found_blank_line = false;

    // Create a copy of the request to avoid modifying the original input string
    char *req_copy = strdup(REQUEST_BUFFER);
    if (!req_copy) {
        perror("strdup");
        exit(1);
    }

    // Open a temporary file to store the request and use fgets on it
    FILE *temp_file = tmpfile();  // Create a temporary file
    if (!temp_file) {
        perror("tmpfile");
        exit(1);
    }

    // Write the request into the temporary file
    fwrite(req_copy, sizeof(char), strlen(req_copy), temp_file);

    // Reset file pointer to the start of the file
    rewind(temp_file);  

    // Use fgets() to read one line at a time
    char line[MALFORMED_FILE_LEN];


    // Iterate through the remaining lines to find the blank line (\r\n)
    while (fgets(line, sizeof(line), temp_file)) {
        // A blank line in HTTP headers is "\r\n", so fgets will return "\r\n"
        if (strcmp(line, "\r\n") == 0) {
            found_blank_line = true;
            break;
        }
    }

    // Close the temporary file and free the request copy
    fclose(temp_file);
    free(req_copy);

    // If either the request is invalid or the blank line wasn't found, print error and exit
    if (!found_blank_line) {
        write(socket, "HTTP/1.1 400 Malformed Request\r\n\r\n", ERROR_LENS); 
        exit(1);
    }
}
/*
The last portion of the request line is “HTTP/VERSION”. The VERSION part of this string is
immaterial for this project and therefore should be ignored. However, you must verify the “HTTP/”
portion is present (case sensitive). If the requested protocol does not start with “HTTP/” then your web
server must return a minimal HTTP response of “HTTP/1.1 501 Protocol Not Implemented\r\n\r\n”.
At this point all processing of the request is finished
*/
void http_protocol_implementation_check(int socket) {
    //char *http_section = strstr(REQUEST_BUFFER, "HTTP/"); 
    if (REQUEST_BUFFER == NULL || strncmp(REQUEST_BUFFER, "HTTP/", PROTOCOL_LEN) != 0) {
        write(socket, "HTTP/1.1 501 Protocol Not Implemented\r\n\r\n", PROT_NO_IMPLEMENT_LEN); 
        exit(1); 
    }
}

/*
Your server will return “HTTP/1.1 405 Unsupported Method\r\n\r\n” if the METHOD portion of
the request is not “GET” or “SHUTDOWN” (case sensitive)

GET /not-there.txt HTTP/1.1\r\n\r\n
*/
void unsupported_method_checker(int socket) {
    bool is_valid = false; 
    if (strncmp(REQUEST_BUFFER, "GET ", GET_LEN) == 0 || strncmp(REQUEST_BUFFER, "SHUTDOWN ", SHUTDOWN_LEN) == 0) {
        is_valid = true; 
    }

    if (is_valid == false) {
        write(socket, "HTTP/1.1 405 Unsupported Method\r\n\r\n", ERROR_LENS); 
        exit(1);    
    }
}

/*
When the “SHUTDOWN” method is requested the “ARGUMENT” is used to authenticate the client.
Your server will take one of the following two approaches:
– When the ARGUMENT given in the HTTP request matches the argument given via the “-t”
option the server will (i) send a minimal HTTP response of “HTTP/1.1 200 Server Shutting
Down\r\n\r\n” and (ii) terminate. Case sensitive matching must be used.
– When the ARGUMENT given in the HTTP request does not match the argument given via
the “-t” option the server will (i) send a minimal HTTP response of “HTTP/1.1 403 Operation
Forbidden\r\n\r\n” and (ii) continue running and accepting further connections and requests.
Case sensitive matching must be used.

./proj3 -p 1947 -t foobar -r ~/doc-root
*/
void server_shutdown(int socket) {
   // Sample shutdown command prefix
    char *shutdown_cmd = "SHUTDOWN ";
    int cmd_len = strlen(shutdown_cmd);

    // Check if the request starts with "SHUTDOWN "
    if (strncmp(REQUEST_BUFFER, shutdown_cmd, cmd_len) != 0) {
        // If the method isn't SHUTDOWN, return
        return;
    }

    // Extract the argument from the request (e.g., "SHUTDOWN /small.txt HTTP/1.1\r\n\r\n")
    char *argument_start = REQUEST_BUFFER + cmd_len;
    
    // Find the space before "HTTP/1.1" to get the end of the argument
    char *argument_end = strstr(argument_start, " HTTP/1.1");

    if (argument_end == NULL) {
        write(socket, "HTTP/1.1 400 Bad Request\r\n\r\n", BAD_REQ_LEN);
        close(socket);
        exit(1); 
    }

    // Calculate the length of the argument
    int argument_len = argument_end - argument_start;

    // Compare the extracted argument with the provided auth_token
    if (strncmp(argument_start, auth_token, argument_len) == 0 && strlen(auth_token) == argument_len) {
        write(socket, "HTTP/1.1 200 Server Shutting Down\r\n\r\n", SERVER_ERR_LEN);
        close(socket); 
        exit(1); 
    } else {
        write(socket, "HTTP/1.1 403 Operation Forbidden\r\n\r\n", SERVER_ERR_LEN);
        close(socket); 
    }
}
/*
When the “GET” method is requested, the “ARGUMENT” will be a filename relative to the “document
root” directory given via the “-r” command-line argument. Your server will use one of these responses:
1. When the requested file does not begin with a “/”, a minimal HTTP response of “HTTP/1.1 406
Invalid Filename\r\n\r\n” must be returned.
2. When the requested file exists, a minimal HTTP response header consisting of “HTTP/1.1 200
OK\r\n\r\n” will be given, followed by the contents of the given file.
3. When the requested file cannot be opened (e.g., because it does not exist), a minimal response
header of “HTTP/1.1 404 File Not Found\r\n\r\n” will be returned (with no payload content).
4. When the requested file is “/” the default file of “/index.html” will be used as the filename. The
web server will then leverage either approach 2 or 3 above based on whether the file exists or not.
Note: This only holds for “/” and not for other directories. E.g., “/foo/” should not return
“/foo/index.html”. To simplify your web server, it explicitly does not need to deal with requests
for directory names (except for “/”)
*/

void get_method_actions(int socket) {

    //get path to file 
    char *file_path_start = strstr(REQUEST_BUFFER, " ") + sizeof(unsigned char); 
    printf("file path start %s\n", file_path_start); 
    char *file_path_end = strstr(REQUEST_BUFFER, " HTTP");
    printf("file path end %s\n", file_path_end);  
    int path_length = file_path_end - file_path_start; 
    printf("path length %i\n", path_length); 
    char path_from_req[path_length];

    strncpy(path_from_req, file_path_start, path_length);

    path_from_req[path_length] = '\0'; 
    printf("this is the buffer holding the file path %s\n", path_from_req); 
    //make sure first character is a /

    if (path_from_req[0] != '/') {
        write(socket, "HTTP/1.1 406 Invalid Filename\r\n\r\n", FILENAME_BAD); 
        close(socket); 
        exit(1); 
    }

    // If the path is "/", use "/index.html"
    if (strcmp(path_from_req, "/") == 0) {
        strcpy(path_from_req, "/index.html");
    }

    //return contents of the file and minimal response header if the file begins with a / 
    //in order to do this, we have to concatenate the root directory and the file 
    char *full_url = malloc(strlen(path_from_req) + strlen(root_directory)); 

    if (full_url == NULL) {
        perror("Failed to allocate memory for full_url");
        close(socket);
        exit(1);
    }

    strcpy(full_url, root_directory); 

    strcat(full_url, path_from_req); 

    //if the file cannot be read, return the 404 error 
    FILE *file = fopen(full_url, "rb"); 

    if (file == NULL) {
        write(socket, "HTTP/1.1 404 File Not Found\r\n\r\n", FILE_NOT_FOUND_LEN); 
        close(socket); 
        exit(1); 
    }
    else {
        write(socket, "HTTP/1.1 200 OK\r\n\r\n", OK_LEN); 

        char buffer[BUFLEN]; 

        while((BYTES_READ = fread(buffer, sizeof(unsigned char), sizeof(buffer), file)) > 0) {
            write(socket, buffer, BYTES_READ); 
        exit(1); 
        }
    fclose(file); 

    free(full_url); 
    close(socket); 

    }

}
void create_tcp_socket() {

    printf("entered the create tcp socket connection\n"); 

    /* determine protocol */
    if ((protoinfo = getprotobyname (PROTOCOL)) == NULL) {
        fprintf(stderr, "cannot find protocol information for.%s\n", PROTOCOL);
        exit(1); 
    }

    printf("protocol determined %s\n", PROTOCOL); 

    /* setup endpoint info */
    memset ((char *)&sock_info,0x0,sizeof (sock_info));
    sock_info.sin_family = AF_INET;
    sock_info.sin_addr.s_addr = INADDR_ANY;
    sock_info.sin_port = htons ((u_short) atoi (port_number));

    printf("socket endpoint information determined\n"); 

    /* allocate a socket */
    /*   would be SOCK_DGRAM for UDP */
    sd = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);
    if (sd < 0) {
        fprintf(stderr, "cannot create socket.\n");
        exit(1); 
    }

    printf("socket sd has been created%i\n", sd); 

    /* bind the socket */
    if (bind (sd, (struct sockaddr *)&sock_info, sizeof(sock_info)) < 0) {
        fprintf(stderr, "cannot bind to port %s", port_number);
        exit(1); 
    }

    printf("socket sd has been bound\n"); 

    /* listen for incoming connections */
    if (listen (sd, QLEN) < 0) {
        fprintf(stderr, "cannot listen on port %s\n", port_number);
        exit(1); 
    }

    printf("listening for incoming connections\n"); 

    /* accept a connection */
    addrlen = sizeof (addr);

    printf("connection has been accepted\n"); 

    while(true) {
    sd2 = accept (sd,&addr,&addrlen);
    if (sd2 < 0) {
        fprintf(stderr, "error accepting connection.\n");
        exit(1); 
        }

    printf("sd2 has been initialized successfully\n"); 

    /* read information from sd2 to get the HTTP request */ 
    int BYTES_READ = read(sd2, REQUEST_BUFFER, REQ_BUF_LEN - sizeof(unsigned char)); 
    REQUEST_BUFFER[BYTES_READ + 1] = '\0'; 
    printf("request buffer is: %s\n", REQUEST_BUFFER); 
    printf("bytes read is: %i\n", BYTES_READ); 
    //check if request is malformed 
    malformed_request_checker(sd2); 
    http_protocol_implementation_check(sd2);
    unsupported_method_checker(sd2);
    if (strncmp(REQUEST_BUFFER, "GET ", GET_LEN) == 0) {
                get_method_actions(sd2);
        } 
    else if (strncmp(REQUEST_BUFFER, "SHUTDOWN ", SHUTDOWN_LEN) == 0) {
                server_shutdown(sd2);
        }

    //close client socket 
    close (sd2); 
    }

    //close server socket 
    close (sd);
}

int main(int argc, char *argv[]) {

    parseargs(argc,argv);
    create_tcp_socket(); 
    return 0; 
}
