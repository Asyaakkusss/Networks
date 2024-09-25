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

void usage (char *progname)
{
    fprintf (stderr,"%s [-i] [-q] [-a] -u URL -w filename [-r]\n", progname);
    fprintf (stderr, "   -i    Debugging Information\n"); 
    fprintf (stderr, "   -q    HTTP Request\n"); 
    fprintf (stderr, "   -a    HTTP Response Header\n"); 
    fprintf (stderr, "   -u U  URL 'U'\n"); 
    fprintf (stderr, "   -w X  filename 'X\n"); 
    fprintf (stderr, "   -r    Redirections from Web Server\n");
    exit (1);
}


void parseargs (int argc, char *argv [])
{
    int opt;

    while ((opt = getopt (argc, argv, "iqau:w:r")) != -1)
    {
        switch (opt)
        {
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
            default:
              usage (argv [0]);
        }
    }

    if ((cmd_line_flags & (ARG_I | ARG_Q | ARG_A)) == 0 || (cmd_line_flags & ARG_U) == 0 || (cmd_line_flags & ARG_W) == 0)    {
        fprintf (stderr,"error: no command line option given\n");
        usage (argv [0]);
    }
}


int main(int argc, char *argv[]) {

    parseargs(argc,argv);

        return 0; 

}