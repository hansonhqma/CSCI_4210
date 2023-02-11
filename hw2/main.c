/*
* CSCI 4210 homework 2
* solves knights tour problem with multiprocessing
* hanson.hq.ma@gmail.com
*/


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

#define move_count 8

void print_invalidusage(){
    perror("ERROR: Invalid argument(s)\n");
    perror("USAGE: hw2.out <m> <n> <r> <c>\n");
}

int pos_in_bounds(int m, int n, int r, int c){

    return m < r && n < c;
}

int main(int argc, char** argv){

    // see if all the arguments are there
    if(argc!=5){
        print_invalidusage();
        return 1;
    }
    // validate arguments
    for(int argi=1;argi<5;++argi){
        char* ptr = argv[argi];
        while((*ptr) != '\0'){
            if(!isdigit(*ptr)){
                print_invalidusage();
                return 1;
            }
            ptr++;
        }
    }

    // we're good to go 👍

    int rows = atoi(argv[1]);
    int cols = atoi(argv[2]);
    int start_m = atoi(argv[3]);
    int start_n = atoi(argv[4]);

    if(!pos_in_bounds(start_m, start_n, rows, cols)){
        print_invalidusage();
        return 1;
    }
    
    // set up pipe
    int* fd = calloc(2, sizeof(int));
    if(pipe(fd)==-1){
        return -1;
    }

    // runtime variables
    int current_m = start_m;
    int current_n = start_n;

    int responsibility = 1;

    int* subprocess_log; // this guy is just going to be int[8]... prioitize speed over memory usage
    
    // create state
    // '\0': have not moved here
    // '1': have been here

    char** state;
    int depth = 0;
    

    while(responsibility){
        // while i have the responsibility of spawning processes...
        
        if(depth>1){
            // free copied-on-write memory if we're not top-level process
            //
            free(subprocess_log);
            subprocess_log = NULL;

            for(int r=0;r<rows;++r){
                free(*(state+r));
            }
            free(state);
            state = NULL;
        }

        if(depth == rows*cols){
            // if there are as many occupied spots as there are moves... i must be done
            printf("tour found by %d, last pos: %d, %d\n", getpid(), current_m, current_n);

            // check if open or closed
            // report to top-level through pipe

            exit(depth);
        }

        // alloc state and subprocess log memory
        state = calloc(rows, sizeof(char*));
        for(int r=0;r<rows;++r){
            *(state+r) = calloc(cols, sizeof(char));
        }

        state[current_m][current_n] = '1';
        depth++;

        subprocess_log = calloc(move_count, sizeof(int));

        // find moves

    }


    


    return 0;
}
