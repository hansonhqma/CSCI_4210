#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>

extern long next_thread_number;
extern int max_squares;
extern int total_open_tours;
extern int total_closed_tours;

void print_invalidusage(){
    fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: hw2.out <m> <n> <r> <c>\n");
}

int pos_in_bounds(int cr, int cc, int r, int c){
    return cr < r && cc < c && cr >= 0 && cc >= 0;
}

struct state{
    int depth;
    // TODO complete
};

void threaded_solve(void *args){

}

int simulate( int argc, char ** argv ){

    // see if all the arguments are there
    if(argc!=5){
        print_invalidusage();
        return 1;
    }
    // validate arguments
    for(int argi=1;argi<5;++argi){
        char* ptr = *(argv+argi);
        while((*ptr) != '\0'){
            if(!isdigit(*ptr)){
                print_invalidusage();
                return 1;
            }
            ptr++;
        }
    }

    // we're good to go 👍
    int quiet = 0;
#ifdef QUIET
    quiet = 1;
#endif

    int no_parallel = 0;
#ifdef NO_PARALLEL
    no_parallel = 1;
#endif

    int rows = atoi(*(argv+1));
    int cols = atoi(*(argv+2));
    int start_row = atoi(*(argv+3));
    int start_col = atoi(*(argv+4));

    if(!pos_in_bounds(start_row, start_col, rows, cols)){
        print_invalidusage();
        return 1;
    }



    return 0;
}
