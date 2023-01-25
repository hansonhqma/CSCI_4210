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

#define MAX_BUFFER_LENGTH 0x100 // 256 chars max buffer length
#define LEFT_BRACE 0x5b
#define RIGHT_BRACE 0x5d


int main(int argc, char** argv){

    if(argc<3){
        fprintf(stderr, "Invalid usage");
        return EXIT_FAILURE;
    }

    char* argcharptr;
    for(argcharptr=*(argv+1); *argcharptr!='\0'; ++argcharptr){
        if(!isdigit(*argcharptr)){
            fprintf(stderr, "Invalid cache size");
            return EXIT_FAILURE;
        }
    }
        
    // read cli arguments

    int input_file_count = argc - 2;

    // initialize some data variables
    int cache_size = atoi(*(argv+1));                   // size of cache
    int* size_array = calloc(cache_size, sizeof(int));  // make size info array
    unsigned int** cache = calloc(cache_size, sizeof(int*));     // cache array, all caches are NULL

    char* buffer = calloc(MAX_BUFFER_LENGTH, sizeof(char));

    for(int current_file_index = 0;
            current_file_index<input_file_count;
            ++current_file_index){ // for every input file

        char* filename = *(argv+current_file_index+2);
        FILE* textfile = fopen(filename, "r");

        if(textfile==NULL){
            perror("File opening failed");
            return EXIT_FAILURE;
        }

        while(fscanf(textfile, "%s", buffer)!=-1){ // for every scanned string

            char* current_char_ptr = buffer;
            while(*current_char_ptr){ // for every char in that string
                if(isdigit(*current_char_ptr)){ // found unsigned number
                    unsigned int current_value = strtol(current_char_ptr, &current_char_ptr, 10);

                    // do caching stuff
                    int cache_idx = current_value % cache_size;
                    int entry_size = *(size_array+cache_idx);
                    if(*(size_array+cache_idx)==0){                // cache at position is empty, calloc
                        *(cache+cache_idx) = calloc(1, sizeof(unsigned int));
                        printf("Read %u => cache index %d (calloc)\n", current_value, cache_idx);

                    }else{ // some stuff already exists there
                        // check for the duplicate first
                        int duplicate = 0;
                        for(int i=0;i<entry_size;++i){
                            unsigned int value_at_cache_entry = *(*(cache+cache_idx) + i);
                            if(value_at_cache_entry == current_value){
                                printf("Read %u => cache index %d (skipped)\n", current_value, cache_idx);
                                duplicate = 1;
                                break;
                            }
                        }
                        if(duplicate){continue;}
                        *(cache+cache_idx) = realloc(*(cache+cache_idx), (entry_size+1)*sizeof(unsigned int));
                        printf("Read %u => cache index %d (realloc)\n", current_value, cache_idx);
                    }
                    *(size_array+cache_idx) += 1;
                    *(*(cache+cache_idx) + entry_size) = current_value;
                }else{
                    current_char_ptr++;
                }
            }
        }
        fclose(textfile);
    }


    char* line_separator = calloc(41, sizeof(char));
    memset(line_separator, '=', 40);
    printf("%s\n", line_separator);

    for(int i=0;i<cache_size;++i){
        if(*(size_array+i) > 0){
            printf("Cache index %d => %c ", i, LEFT_BRACE);
            for(int j=0;j<*(size_array+i);++j){
                printf("%u", *(*(cache+i)+j));
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

    return EXIT_SUCCESS;
}
