/*
 * =====================================================================================
 *
 *       Filename:  hw1.c
 *
 *    Description:  cache mechanism
 *
 *        Version:  1.0
 *        Created:  01/18/2023 06:56:45 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_BUFFER_LENGTH 16 // 128 chars max buffer length


int main(int argc, char** argv){

    // read cli arguments

    if(argc!=3){
        perror("invalid usage\n");
        return EXIT_FAILURE;
    }

    int cache_size = atoi(*(argv+1));
    char* filename = *(argv+2);

    printf("cache_size: %d\nfilename: %s\n", cache_size, filename);

    char* buffer = calloc(MAX_BUFFER_LENGTH, sizeof(char));
    int intbuffer = 0;
    FILE* textfile = fopen(filename, "r");
    
    if(textfile==NULL){
        perror("File opening failed\n");
        return EXIT_FAILURE;
    }

    intbuffer = getw(textfile);

    printf("%d\n", intbuffer);







    return EXIT_SUCCESS;
}
