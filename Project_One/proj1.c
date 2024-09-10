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
    FILE *file = fopen("samplec.txt","r"); 

    //ensure throwing error if the file is not found 
    if (file == NULL) {
        printf("file is not available. Please try another file name"); 
    }
    
    //allocate memory for a buffer that reads information from the file into the buffer 
    char buffer[16]; 

    //counter for valid and invalid addresses 
    int valid_counter = 0; 
    int invalid_counter = 0; 

    //fgets is a method that reads each string of the buffer up until the newline 
    while (fgets(buffer, sizeof(buffer), file)) {

        //tokenize the string based on the dots present 
        char *pointer = strtok(buffer, "."); 

        //counter for number of octets
        int num_octets = 0; 

        //boolean for is valid 
        int is_valid = 1; 

        while (pointer) {
            //check that there are no leading zeroes 
            if (pointer[0] == '\0' || (pointer[0] == '0' && pointer[1] == '\0')) {
                is_valid = 0; 
                break; 
            }
            /*check to make sure that each address is made of four integer values
            more specifically: 
            1. can be any 1-digit number 0-9
            2. can be any 2-digit number 10-99
            3. can be any 3-digit number 100-255 
            4. canNOT be more than 3 digits 
            */
            int octet = atoi(pointer); 
                
            if (octet > 255 || octet <= 0) {
                is_valid = 0; 
                break; 
            }
                

            //check that there are four quads (make sure there are 3 periods present. this should be enough to pass this edge case)

            //check that each IP address begins with a number from 1-9

            //move on to the next token 
            
        }
        printf("%s", buffer); 
        valid_counter += valid_counter + 1; 
    }

    fclose(file); 

    return 0; 
}