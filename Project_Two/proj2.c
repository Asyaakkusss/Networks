/*
-i option
The “-i” option will be used to print debugging information about the given command line parameters to
standard output (i.e., the screen). When “-i” is given on the command line your program will output the
following lines:
INFO: host: [hostname]
INFO: web_file: [url_filename]
INFO: output_file: [local_filename]
The format for these lines must follow these requirements:
• The “INFO:” must be at the beginning of the line.
• A single space follows “INFO:”.
• The labels that appear after “INFO:<space>” must be exactly as they appear above (e.g., using all
lower case letters).
• After the label, add a colon (“:”) and then a single space before printing the value.
• The “[hostname]” value comes from the URL given on the command line, as described in the “-u”
discussion above.
• The “[url filename]” value is the filename portion of the URL given on the command line, as described
in the “-u” discussion above. If no filename is given on the command line, the default filename of “/”
must be printed.
• The “[local filename]” value is the name of the file on the local system where the web page at the given
URL will be stored. This is the filename given with the “-w” option on the command line.
• The three lines must appear in the order given above.
• Do not print extra lines—including blank lines.
• Do not print any extra whitespace before, after or within the output lines.
• The “-i” output is to be printed regardless of whether there are errors fetching the web page. The “-i”
output will not be printed if there are errors in the command line options given by the user.
The following are several illustrative examples with the “-i” option:
%%% ./proj2 -i -u http://www.icir.org -w testing.html
INFO: host: www.icir.org
INFO: web_file: /
INFO: output_file: testing.html
%%% ./proj2 -w mallman.html -i -u http://www.icir.org/mallman/
INFO: host: www.icir.org
INFO: web_file: /mallman/
INFO: output_file: mallman.html
%%% ./proj2 -u http://www.icir.org/mallman/index.html -w /tmp/mallman.html -i
INFO: host: www.icir.org
INFO: web_file: /mallman/index.html
INFO: output_file: /tmp/mallman.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ARG_I  0x1  //1
#define ARG_Q  0x2  //2
#define ARG_A  0x4  //4
#define ARG_U  0x8  //8
#define ARG_W  0x10 //16
#define ARG_R  0x20 //32

unsigned short cmd_line_flags = 0;
char *url = NULL;
char *filename = NULL; 

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

void i_option() {
  //grab the command line
  //print out INFO: host: the thing after http 
  //print out INFO: web_file: print out / if no slash found, print out rest of string if it exsits 
  //print out INFO: output_file: print out whatever is after -w 
  char host[strlen(url)]; 
  char web_file[strlen(url)]; 


  //find the :// portion and fast forward 3 places 
  const char* start = strstr(url, "://") + 3; //pseudo-start of the host (we have to add 3 to this)

  //start a new leaf...find the first / in order to parse the web file portion 
  const char* end = strchr(start, '/'); 

  //strncpy end - start amount of characters from the string beginning from the start pointer 
  if (end != NULL) {
    strncpy(host, start, end - start); 
    host[end-start] = '\0'; 
  }
  //strcpy the entire string from the start character to the end if there is no web file 
  else {
    strcpy(host, start); 
  }

  //for the web file, strcpy the entire string from start to end beginning from the end pointer 
  if (end != NULL) {
  strcpy(web_file, end); 
  }

  //if there is no web file, return a / and then null terminate the string so no extra garbage is printed 
  else {
    web_file[0] = '/'; 
    web_file[1] = '\0'; 
  }

  printf("INFO: host: %s\n", host); 
  printf("INFO: web_file: %s\n", web_file); 
  printf("INFO: output_file: %s\n", filename); 
}

int main(int argc, char *argv[]) {

    parseargs(argc,argv);

    if (cmd_line_flags == ARG_I+ARG_U+ARG_W) {
      i_option(); 
    }

    return 0; 

}