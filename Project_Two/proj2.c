#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


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

unsigned short cmd_line_flags = 0;
char *url = NULL;
char *filename = NULL; //file name for the -w portion 

char HOST_NAME[MAX_LENGTH]; //create global variable for the host name 
char URL_FILENAME[MAX_LENGTH]; //file name from the url 

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

/*comment this out for now to focus on the q option 
void a_option() {
    struct sockaddr_in sock_addr_info; 
    struct hostent *hinfo;
    struct protoent *protoinfo;
    char buffer [BUFLEN];
    int sd, ret;

    //lookup the hostname
    hinfo = gethostbyname (find_host());

    //set endpoint information 
    memset ((char *)&sock_addr_info, 0x0, sizeof (sock_addr_info)); //set aside memory for socket address structure 
    sock_addr_info.sin_family = AF_INET; //address family is internet 
    sock_addr_info.sin_port = htons (atoi (argv [PORT_POS])); //set port number in network byte order 
    memcpy ((char *)&sock_addr_info.sin_addr,hinfo->h_addr,hinfo->h_length); //copy ip address from the host info to the sock addr info struct 

    sd = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto); //create a socket 
    
    //connect the socket
    connect (sd, (struct sockaddr *)&sock_addr_info, sizeof(sock_addr_info));

    //snarf whatever server provides and print it
    memset (buffer,0x0,BUFLEN);
    ret = read (sd,buffer,BUFLEN - 1);
    if (ret < 0)
        errexit ("reading error",NULL);
    fprintf (stdout,"%s\n",buffer);
            
    //close & exit
    close (sd);
}
*/

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

    return 0; 

}