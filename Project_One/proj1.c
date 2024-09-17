/*
Name: Asya Akkus 
Case Network ID: aya29
File Name: proj1.c
Date Created: 9/4/2024
Description: file parses long lists of IP addresses and 
determines which ones are valid. It returns them in list
or summary mode, depending on what the user chooses in 
the command line. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ARG_SUMMARY  0x1
#define ARG_LIST     0x2
#define ARG_FILE     0x4

unsigned short cmd_line_flags = 0;
char *filename = NULL;


void usage (char *progname)
{
    fprintf (stderr,"%s [-s] [-l] [-f filename]\n", progname);
    fprintf (stderr,"   -s    summary mode\n");
    fprintf (stderr,"   -l    list mode\n");
    fprintf (stderr,"   -f X  file name 'X'\n");
    exit (1);
}


void parseargs (int argc, char *argv [])
{
    int opt;

    while ((opt = getopt (argc, argv, "slf:")) != -1)
    {
        switch (opt)
        {
            case 's':
              cmd_line_flags |= ARG_SUMMARY;
              break;
            case 'l':
              cmd_line_flags |= ARG_LIST;
              break;
            case 'f':
              cmd_line_flags |= ARG_FILE;
              filename = optarg;
              break;
            case '?':
            default:
              usage (argv [0]);
        }
    }

    if ((cmd_line_flags & (ARG_SUMMARY | ARG_LIST)) == 0 || (cmd_line_flags & ARG_FILE) == 0)    {
        fprintf (stderr,"error: no command line option given\n");
        usage (argv [0]);
    }
}

void summary_mode() {
    FILE* file = fopen(filename, "r"); 

    // Ensure throwing error if the file is not found 
    if (file == NULL) {
        printf("File is not available. Please try another file name\n");
    }
    
    // Allocate memory for a buffer that reads information from the file into the buffer 
    char buffer[50]; 

    // Counters for valid and invalid addresses 
    int valid_counter = 0; 
    int invalid_counter = 0; 

    // Read each line from the file
    while (fgets(buffer, sizeof(buffer), file)) {

        // Remove trailing newline character, if present
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // Boolean for is valid 
        int is_valid = 1; 

        //counter for number of dots 
        int dot_count = 0; 

        //we check how many periods there are before tokenizing it, so we can take it out of the running 
        //if things don't add up. There should only be 3 periods in each IP address. 
        //check to make sure there are only 3 periods in the buffer 
        for (int i = 0; i < strlen(buffer); i++) {
            if (buffer[i] == '.')
                dot_count++; 
        }

        if (dot_count > 3) {
            is_valid = 0; 
        }

        // Tokenize the string based on the dots present 
        char *pointer = strtok(buffer, "."); 

        // Counter for number of octets
        int num_octets = 0; 

        while (pointer) {
            // Check for leading zeroes
            if (pointer[0] == '\0' || (pointer[0] == '0' && pointer[1] != '\0')) {
                is_valid = 0; 
                break; 
            }

            // Check if the octet contains only digits
            for (size_t i = 0; i < strlen(pointer); i++) {
                if (!isdigit((unsigned char)pointer[i])) {
                    is_valid = 0; 
                    break;
                }
            }
            if (!is_valid) break;

            // Convert the octet to an integer and validate its range
            int octet = atoi(pointer); 
            if (octet > 255 || octet < 0) {
                is_valid = 0; 
                break; 
            }

            // Move on to the next token 
            pointer = strtok(NULL, "."); 
            num_octets++; 
        }

        // Validate the IP address based on the number of octets
        if (is_valid && num_octets == 4) {
            valid_counter++; 
        } else {
            invalid_counter++; 
        }
    }

    fclose(file); 

    int num_lines = valid_counter + invalid_counter; 
    printf("LINES: %d\n", num_lines);
    printf("VALID: %d\n", valid_counter);
    printf("INVALID: %d\n", invalid_counter);

}

void list_mode() {

    FILE *file = fopen(filename, "r"); 

    // Ensure throwing error if the file is not found 
    if (file == NULL) {
        printf("File is not available. Please try another file name\n");
    }
    
    // Allocate memory for a buffer that reads information from the file into the buffer 
    char buffer[50]; 

    //Allocate memory to another buffer that stores the value to be spit out in list mode 
    char output_buffer[50]; 

    // Counters for valid and invalid addresses 
    int valid_counter = 0; 
    int invalid_counter = 0; 

    // Read each line from the file
    while (fgets(buffer, sizeof(buffer), file)) {
        //create a new buffer that will be used to 
        //fgets(output_buffer, sizeof(buffer), file); 
         
        memcpy(output_buffer, buffer, 50);

        //remove newline 
        size_t out_len = strlen(output_buffer); 
        if (out_len > 0 && output_buffer[out_len - 1] == '\n') {
            output_buffer[out_len - 1] = '\0'; 
        }

        // Remove trailing newline character, if present
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // Boolean for is valid 
        int is_valid = 1; 

        //counter for number of dots 
        int dot_count = 0; 

        //we check how many periods there are before tokenizing it, so we can take it out of the running 
        //if things don't add up. There should only be 3 periods in each IP address. 
        //check to make sure there are only 3 periods in the buffer 
        for (int i = 0; i < strlen(buffer); i++) {
            if (buffer[i] == '.')
                dot_count++; 
        }

        if (dot_count > 3) {
            is_valid = 0; 
        }

        // Tokenize the string based on the dots present 
        char *pointer = strtok(buffer, "."); 

        // Counter for number of octets
        int num_octets = 0; 

        while (pointer) {
            // Check for leading zeroes
            if (pointer[0] == '\0' || (pointer[0] == '0' && pointer[1] != '\0')) {
                is_valid = 0; 
                break; 
            }

            // Check if the octet contains only digits
            for (size_t i = 0; i < strlen(pointer); i++) {
                if (!isdigit((unsigned char)pointer[i])) {
                    is_valid = 0; 
                    break;
                }
            }
            if (!is_valid) break;

            // Convert the octet to an integer and validate its range
            int octet = atoi(pointer); 
            if (octet > 255 || octet < 0) {
                is_valid = 0; 
                break; 
            }

            // Move on to the next token 
            pointer = strtok(NULL, "."); 
            num_octets++; 
        }

        // Validate the IP address based on the number of octets
        if (is_valid && num_octets == 4) {
            valid_counter++; 
            printf("%s + \n", output_buffer); 
        } else {
            invalid_counter++; 
            printf("%s - \n", output_buffer); 
        }
    }

    fclose(file); 
}

int main(int argc, char *argv[]) {

    parseargs(argc,argv);

    //if the command lines indicate to run in summary mode, run summary_mode() method 
    if (cmd_line_flags == ARG_SUMMARY+ARG_FILE) {
        summary_mode(); 
    }
        
    if (cmd_line_flags == ARG_LIST+ARG_FILE) {
        list_mode(); 
    }
        
    if (cmd_line_flags == ARG_LIST+ARG_SUMMARY+ARG_FILE) {
        fprintf(stderr, "error: summary and list mode cannot be given together\n"); 
        exit (1); 
    }

        return 0; 

}