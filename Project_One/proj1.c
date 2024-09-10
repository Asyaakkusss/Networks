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

int main() {
    FILE *file = fopen("samplec.txt","r"); 
    
    //allocate memory for a buffer that reads information from the file into the buffer 
    char buffer[15]; 

    //fgets is a method that reads each string of the buffer up until the newline 
    while (fgets(buffer, sizeof(buffer), file)) {
        printf("%s", buffer); 
    }

    fclose(file); 

    return 0; 
}