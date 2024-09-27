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


#define ARG_I  0x1  //1
#define ARG_Q  0x2  //2
#define ARG_A  0x4  //4
#define ARG_U  0x8  //8
#define ARG_W  0x10 //16
#define ARG_R  0x20 //32

#define MAX_LENGTH 256

#define A_BUFFER_LEN 1024
#define HOST_POS 8080
#define PORT_POS 80
#define PROTOCOL "tcp"

unsigned short cmd_line_flags = 0;
char *url = NULL;
char *filename = NULL; //file name for the -w portion 

char HOST_NAME[MAX_LENGTH]; //create global variable for the host name 
char URL_FILENAME[MAX_LENGTH]; //file name from the url 
char GET_REQUEST[A_BUFFER_LEN]; 


//global variables for part a 
struct sockaddr_in sock_addr_info; //specifies transport address and port for AF_INET address family 
struct hostent *hinfo; //stores informations about given host, such as name, aliases, address type, length, and address list 
struct protoent *protoinfo; //stores name and protocol numbers corresponding to a given protocol name, such as offical name of protocol, alternate names, and protocol number in host byte order 
int sd, ret;

//./proj2 [-i] [-q] [-a] -u URL -w filename

void usage (char *progname) {
    fprintf (stderr,"%s [-i] [-q] [-a] -u URL -w filename [-r]\n", progname);
    fprintf (stderr, "   -i    Debugging Information\n"); 
    fprintf (stderr, "   -q    HTTP Request\n"); 
    fprintf (stderr, "   -a    HTTP Response Header\n"); 
    fprintf (stderr, "   -u U  URL 'U'\n"); 
    fprintf (stderr, "   -w X  filename 'X'\n"); 
    fprintf (stderr, "   -r    Redirections from Web Server\n");
    exit (1);
}

void validateargs (int argc, char *argv []) {

  
    // The filename must be present
    if ((cmd_line_flags & ARG_W) == 0 || filename == NULL) {
        fprintf(stderr, "error: -w option must be given with a filename\n");
        usage(argv[0]);
    }

    // The URL must be present
    if ((cmd_line_flags & ARG_U) == 0 || url == NULL) {
        fprintf(stderr, "error: -u option must be given with a URL\n"); 
        usage(argv[0]);
    }

}

void parseargs (int argc, char *argv []) {
    int opt;

    while ((opt = getopt (argc, argv, "iqau:w:r")) != -1) {
        switch (opt) {
            case 'i':
              cmd_line_flags |= ARG_I;
              break;
            case 'q':
              cmd_line_flags |= ARG_Q;
              break;
            case 'a':
              cmd_line_flags |= ARG_A;
              break;
            case 'u': 
              cmd_line_flags |= ARG_U; 
              url = optarg; 
              break; 
            case 'w': 
              cmd_line_flags |= ARG_W; 
              filename = optarg; 
              break; 
            case 'r': 
              cmd_line_flags |= ARG_R; 
              break; 
            case '?':
              if (optopt == 'u') {
                    fprintf(stderr, "error: -u option requires a URL\n");
                } else if (optopt == 'w') {
                    fprintf(stderr, "error: -w option requires a filename\n");
                } else {
                    fprintf(stderr, "error: unknown option -%c\n", optopt);
                }
                usage(argv[0]);
                return; // Exit after handling error
            default:
                usage(argv[0]);
                return; // Exit after handling error
        }
    }

    validateargs(argc, argv); 
}

/*for the global variables of host name and url filename*/
void find_host(char *url) {
  const char* start = strstr(url, "://") + 3; 

  const char* end = strchr(start, '/'); 

  if (end != NULL) {
    strncpy(HOST_NAME, start, end-start); 
    HOST_NAME[end - start] = '\0'; 
  }

  else {
    strcpy(HOST_NAME, start); 
  }

}

void find_url_filename(char *url) {
  const char* start = strstr(url, "://") + 3; 

  const char* end = strchr(start, '/'); 

  if (end != NULL) {
  strcpy(URL_FILENAME, end); 
  }

  //if there is no web file, return a / and then null terminate the string so no extra garbage is printed 
  else {
    URL_FILENAME[0] = '/'; 
    URL_FILENAME[1] = '\0'; 
  }

}

//QUESTION: is it ok to create a method that runs the q option stuff and then call that method inside i_option to avoid repeating code because if we call the q option that means we can't call the a option and the q and a 
//options share some functionality in common? 

void create_get_header() {

  /*
  GET [url_filename] HTTP/1.0\r\nHost: [hostname]\r\nCase CSDS 325/425 WebClient 0.1\r\n\r\n
  */

  snprintf(GET_REQUEST, A_BUFFER_LEN, "GET %s HTTP/1.0\r\n" "Host: %s \r\n" "User-Agent: Case CSDS 325/425 WebClient 0.1 \r\n" "\r\n", URL_FILENAME, HOST_NAME); 
}

void a_option() {

    //create http get request (stored as a global variable called GET_REQUEST. )
    create_get_header(); 

    char response_header_buffer[A_BUFFER_LEN]; 

    hinfo = gethostbyname(HOST_NAME); 
    if (hinfo == NULL) {
      fprintf(stderr, "cannot find host name %s\n", HOST_NAME); 
      exit(1); 
    }

    //set endpoint information
    memset ((char *)&sock_addr_info, 0x0, sizeof (sock_addr_info)); //casting address of sock_addr_info to a char, sets all bytes in block to 0, specifies how many bytes of memory should be set to 0 
    sock_addr_info.sin_family = AF_INET; //family of sockets set to af_inet 
    sock_addr_info.sin_port = htons(PORT_POS); //socket port position given and converted to network byte order 
    memcpy ((char *)&sock_addr_info.sin_addr,hinfo->h_addr,hinfo->h_length); //destination (sock_addr_info_addr value of sock_addr_info struct), source from hostent, and number of bytes to copy 

    if ((protoinfo = getprotobyname(PROTOCOL)) == NULL) {
        fprintf(stderr, "cannot find protocol information for %s", PROTOCOL);
        exit(1); 
    }

    //allocate socket 
    sd = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);
    if (sd < 0) {
        fprintf(stderr, "cannot create socket");
        exit(1);
    }

    //connect the socket 
    if (connect(sd, (struct sockaddr *)&sock_addr_info, sizeof(sock_addr_info)) < 0) {
        fprintf (stderr, "cannot connect");
        exit(1); 
    }

    //send request 
    send(sd, GET_REQUEST, strlen(GET_REQUEST), 0);
    //read response 
    memset(response_header_buffer, 0, A_BUFFER_LEN);  
    read(sd, response_header_buffer, A_BUFFER_LEN - 1); 


    //we need to truncate, so we make a pointer to the end of the header and terminate the string there 
    char *header_end = strstr(response_header_buffer, "\r\n\r\n"); 
    if (header_end != NULL) {

    *header_end = '\0'; 
    }
    //print response header 
    fprintf(stdout, "%s\n", response_header_buffer); 
    
    /* close & exit */
    close (sd);
    exit (0);
}

void i_option() { 
  
  printf("INFO: host: %s\n", HOST_NAME); 
  printf("INFO: web_file: %s\n", URL_FILENAME); 
  printf("INFO: output_file: %s\n", filename); 
}


void q_option() {

  printf("REQ: GET %s HTTP/1.0\r\n", URL_FILENAME); 
  printf("REQ: Host: %s \r\n", HOST_NAME); 
  printf("REQ: User-Agent: Case CSDS 325/425 WebClient 0.1 \r\n"); 
  printf("\r\n"); 
}

int main(int argc, char *argv[]) {

    parseargs(argc,argv);

    find_host(url); 
    find_url_filename(url); 
    if (cmd_line_flags == ARG_I+ARG_U+ARG_W) {
      i_option(); 
    }

    if (cmd_line_flags == ARG_Q+ARG_U+ARG_W) {
      q_option(); 
    }

    if (cmd_line_flags == ARG_A+ARG_U+ARG_W) {
      a_option(); 
    }

    return 0; 

}