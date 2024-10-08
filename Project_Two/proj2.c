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
#define W_BUFFER_LEN 20
#define HOST_POS 8080
#define PORT_POS 80
#define PROTOCOL "tcp"
#define RESP_TYPE_LEN 4
#define IDEAL_RESP_NO 200
#define REDIRECT_RESP 301
#define REQUEST_TYPE_PTR_ORIENTATION 9 
#define URL_PTR_ORIENTATION 3
#define LOCATION_PTR_ORIENTATION 10 
#define TOREAD_CONTENT_PTR 4 
unsigned short cmd_line_flags = 0;
char *url = NULL;
char *filename = NULL; //file name for the -w portion 

char HOST_NAME[MAX_LENGTH]; //create global variable for the host name 
char URL_FILENAME[MAX_LENGTH]; //file name from the url 
char URL_REDIRECT[MAX_LENGTH]; //create global variable for both the host name and url for processing the redirect request 
char GET_REQUEST[A_BUFFER_LEN]; 
char RESPONSE_HEADER_BUFFER[A_BUFFER_LEN]; 


//global variables for part a 
struct sockaddr_in sock_addr_info; //specifies transport address and port for AF_INET address family 
struct hostent *hinfo; //stores informations about given host, such as name, aliases, address type, length, and address list 
struct protoent *protoinfo; //stores name and protocol numbers corresponding to a given protocol name, such as offical name of protocol, alternate names, and protocol number in host byte order 
int sd, ret;
int BYTES_READ; 

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


int find_response_type(char *req_buffer) {
  //make variable that holds response type 
  char response_type[RESP_TYPE_LEN]; 

  //find the first character of the request type in the header
  char *request_type = strstr(req_buffer, "HTTP/1.1") + REQUEST_TYPE_PTR_ORIENTATION;

  //find first occurence of a newline character in the new request_type string 
  char *end = strchr(request_type, ' '); 

  int length = end - request_type; 
  strncpy(response_type, request_type, length); 
  response_type[length] = '\0'; 
  
  //validate the response number from the response type 
  int response_number = atoi(response_type); 
  
  return response_number; 
}
/*for the global variables of host name and url filename*/
void find_host(char *url) {
  const char* start = strstr(url, "://") + URL_PTR_ORIENTATION; 

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
  const char* start = strstr(url, "://") + URL_PTR_ORIENTATION; 

  const char* end = strchr(start, '/'); 

  if (end != NULL) {
  strcpy(URL_FILENAME, end); 
  }

  //if there is no web file, return a / and then null terminate the string so no extra garbage is printed 
  else {
    //URL_FILENAME[0] = '/'; 
    //URL_FILENAME[1] = '\0'; 
    strcpy(URL_FILENAME, "/\0"); 
  }

}

//QUESTION: is it ok to create a method that runs the q option stuff and then call that method inside i_option to avoid repeating code because if we call the q option that means we can't call the a option and the q and a 
//options share some functionality in common? 

void create_get_header() {

  snprintf(GET_REQUEST, A_BUFFER_LEN, "GET %s HTTP/1.0\r\n" "Host: %s\r\n" "User-Agent: Case CSDS 325/425 WebClient 0.1\r\n" "\r\n", URL_FILENAME, HOST_NAME); 
}

void create_socket() {
    //create http get request (stored as a global variable called GET_REQUEST. )
    create_get_header();  //instead of doing that, do not call create get header 

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
    memset(RESPONSE_HEADER_BUFFER, 0, A_BUFFER_LEN);  
    
    BYTES_READ = read(sd, RESPONSE_HEADER_BUFFER, A_BUFFER_LEN - 1); //try to read this line by line use fgets (dump web object written to local file through -w)

}

void w_option() {
  /*w: write the entire website onto a file. at the end of the run, you need to have a file. w has no output to the screen. 
  make sure it is 200. keep reading until you get to the end of a file/socket gets closed.*/

  //creates socket and writes HTTP response header to the RESPONSE_HEADER_BUFFER variable 
  create_socket(); 

  //we have to first ensure that the request is a 200 type and then print a meaningful error message if it isnt. we parse the first part of the header in order to ascertain this 
  int resp = find_response_type(RESPONSE_HEADER_BUFFER);
 
  if (resp != IDEAL_RESP_NO && resp != REDIRECT_RESP) {
    fprintf(stderr, "the code for this website is not an OK code of 200. Please try another link\n"); 
    exit(1); 
  }

  //we find the start of the content when we hit \r\n\r\n
  char *content_tobe_read = strstr(RESPONSE_HEADER_BUFFER, "\r\n\r\n") + TOREAD_CONTENT_PTR; 
  FILE *sockptr = fdopen(sd, "rb"); 
  //write everything into a file 
  FILE *w_file; 
  w_file = fopen(filename, "wb"); 
  
  //number of bytes between start and end of header 
  int header_end = content_tobe_read - RESPONSE_HEADER_BUFFER; 

  //remaining header bytes without the starting response block of text
  int remaining_header_bytes = BYTES_READ - header_end; 
  if (remaining_header_bytes > 0) {
    fwrite(content_tobe_read, sizeof(unsigned char), remaining_header_bytes, w_file); 
  }

  
 
  // Continue reading the rest of the content from the socket.
  char buffer[W_BUFFER_LEN];
  int bytes_read;
  while ((bytes_read = fread(buffer, sizeof(unsigned char), W_BUFFER_LEN, sockptr)) > 0) {
      fwrite(buffer, 1, bytes_read, w_file);
    }
  fclose(w_file); 

  //we write everything after this to the file  

  //we write everything after this to the file 
}

void a_option() {

    create_socket(); 

    //we need to truncate, so we make a pointer to the end of the header and terminate the string there 
    char *header_end = strstr(RESPONSE_HEADER_BUFFER, "\r\n\r\n"); 
    if (header_end != NULL) {
    *header_end = '\0'; 
    }
    
    char *tokenized_string = strtok(RESPONSE_HEADER_BUFFER, "\r\n"); 

    /*we want RSP: to be appended to every line*/
    while (tokenized_string) {
      printf("RSP: %s\r\n", tokenized_string); 

      tokenized_string = strtok(NULL, "\r\n"); 
    }
    
    //close and exit 
    close (sd);

    //w_option(); 
}

void i_option() { 
  create_socket(); 
  printf("INFO: host: %s\n", HOST_NAME); 
  printf("INFO: web_file: %s\n", URL_FILENAME); 
  printf("INFO: output_file: %s\n", filename); 
}

void q_option() {
  create_socket();

  //separate string based on \r\n on each line 
  char *tokenized_get = strtok(GET_REQUEST, "\r\n");  
  while (tokenized_get != NULL) {
    printf ("REQ: %s\r\n",tokenized_get);
    tokenized_get = strtok (NULL, "\r\n");
  }
}

/*In this case, the desired web page is not at the URL given on the command line (“http://www.icir.org/mark/”).
Rather, the web server uses a response with a code of 301 to redirect the client to a different URL—which
is given in the “Location:” line of the response header. When the “-r” option is given, the web client
will download the URL given in the redirection message. Redirection can happen more than once. E.g.,
“www.foo.com” could redirect to “www.bar.com” which could in turn redirect to “bar.com”. When redirects
happen and the “-q” or “-a” options are given the client must print every request and/or response header
encountered in the order encountered. The output file given with “-w” will contain the contents of the
ultimate (last) response. An example:*/
/*
void r_option() {
  create_socket(); 
  int resp = find_response_type(RESPONSE_HEADER_BUFFER); 
  char buffer[250]; 
  if (resp != 301) {
    exit(1); 
  }

  //if the response is 301, then enter a loop. run the loop until the extracted response is 201
  
  while (resp == 301) {
  //find string with location in the response header and move the pointer to the first character to the front of the url 
  char *location_string = strstr(RESPONSE_HEADER_BUFFER, "Location") + LOCATION_PTR_ORIENTATION; 

  int i = 0; 
  while (*location_string != '\r' && *location_string != '\n' && i < sizeof(buffer) - 1) {
    buffer[i] = location_string; 
    location_string++; 
    i++; 
  }

  find_host(buffer); 
  find_url_filename(buffer); 
  }

}*/


int main(int argc, char *argv[]) {

    parseargs(argc,argv);

    find_host(url); 
    find_url_filename(url); 
    if (cmd_line_flags == ARG_I+ARG_U+ARG_W) {
      w_option(); 
      i_option(); 
    }

    if (cmd_line_flags == ARG_Q+ARG_U+ARG_W) {
      w_option(); 
      q_option(); 
    }

    if (cmd_line_flags == ARG_A+ARG_U+ARG_W) {
        w_option();
        a_option(); 
    }

    if (cmd_line_flags == ARG_U+ARG_W) {
      w_option(); 
    }

    if (cmd_line_flags == ARG_U+ARG_Q+ARG_W+ARG_R) {
      w_option(); 
      q_option(); 
    }

    if (cmd_line_flags == ARG_U+ARG_W+ARG_R+ARG_A) {
      w_option(); 
      a_option(); 
      //r_option(); 
    }

    return 0; 

}