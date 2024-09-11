/*
Name: Asya Akkus 
Case Network ID: aya29
File Name: proj1
Date Created: 9/4/2024
Description: file parses long lists of IP addresses and 
determines which ones are valid. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main() {
    FILE *file = fopen("/home/asyaakkus/Networks/sample-G-input.txt", "r"); 

    // Ensure throwing error if the file is not found 
    if (file == NULL) {
        printf("File is not available. Please try another file name\n");
        return 1;  // Exit with an error code
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

    return 0; 
}