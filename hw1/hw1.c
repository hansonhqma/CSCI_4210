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
#include <ctype.h>
#include <string.h>

#define MAX_BUFFER_LENGTH 16 // 128 chars max buffer length
#define LEFT_BRACE 91
#define RIGHT_BRACE 93


int main(int argc, char** argv){

    // read cli arguments

    if(argc!=3){
        perror("invalid usage");
        return EXIT_FAILURE;
    }

    char* filename = *(argv+2);

    char charbuffer;
    FILE* textfile = fopen(filename, "r");
    
    if(textfile==NULL){
        perror("File opening failed");
        return EXIT_FAILURE;
    }

    // initialize some data variables
    int cache_size = atoi(*(argv+1));                   // size of cache
    int* size_array = calloc(cache_size, sizeof(int));  // make size info array
    int** cache = calloc(cache_size, sizeof(int*));     // cache array, all caches are NULL


    char* buffer = calloc(MAX_BUFFER_LENGTH, sizeof(char));

    while(!feof(textfile)){
        fscanf(textfile, "%s", buffer);

        char* current_char_ptr = buffer;
        while(*current_char_ptr){ // keep reading chars
            if(isdigit(*current_char_ptr)){ // found unsigned number
                int current_value = (int)strtol(current_char_ptr, &current_char_ptr, 10); // smash to int

                // do caching stuff
                int cache_idx = current_value % cache_size - 1;
                int entry_size = *(size_array+cache_idx);
                if(*(size_array+cache_idx)==0){                // cache at position is empty, calloc
                    *(cache+cache_idx) = calloc(1, sizeof(int));
                    printf("Read %d => cache index %d (calloc)\n", current_value, cache_idx+1);

                }else{ // some stuff already exists there
                    // check for the duplicate first
                    int duplicate = 0;
                    for(int i=0;i<entry_size;++i){
                        int value_at_cache_entry = *(*(cache+cache_idx) + i);
                        if(value_at_cache_entry == current_value){
                            printf("Read %d => cache index %d (skipped)\n", current_value, cache_idx+1);
                            duplicate = 1;
                            break;
                        }
                    }
                    if(duplicate){continue;}
                    *(cache+cache_idx) = realloc(*(cache+cache_idx), (entry_size+1)*sizeof(int));
                    printf("Read %d => cache index %d (realloc)\n", current_value, cache_idx+1);
                }
                *(size_array+cache_idx) += 1;
                *(*(cache+cache_idx) + entry_size) = current_value;
            }else{
                current_char_ptr++;
            }
        }
    }


    // TODO: print stuff

    char* line_separator = calloc(41, sizeof(char));
    memset(line_separator, '=', 40);
    printf("%s\n", line_separator);

    for(int i=0;i<cache_size;++i){
        if(*(size_array+i) > 0){
            printf("Cache index %d => %c ", i+1, LEFT_BRACE);
            for(int j=0;j<*(size_array+i);++j){
                printf("%d", *(*(cache+i)+j));
                if(j!=*(size_array+i)-1){
                    printf(", ");
                }
            }
            printf(" %c\n", RIGHT_BRACE);
        }
    }

    // free memory
    for(int i=0;i<cache_size;++i){
        free(*(cache+i));
    }
    free(cache);
    free(size_array);
    free(line_separator);

    free(buffer);
    fclose(textfile);


    return EXIT_SUCCESS;
}
