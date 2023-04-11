#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>

#define EIGHT 8

extern long next_thread_number;
extern int max_squares;
extern int total_open_tours;
extern int total_closed_tours;

int rows, cols, max_depth;

int start_row, start_col;

pthread_mutex_t sync_lock;

void print_invalidusage(){
    fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: hw2.out <m> <n> <r> <c>\n");
}

int get_row_delta(int move_identifier){
    // returns change in row based on move identifier
    int rd = 1;
    if(move_identifier == 0 || move_identifier == 3 || move_identifier == 4 || move_identifier == 7){
        rd = 2;
    }
    if(move_identifier < 2 || move_identifier > 5){
        rd *= -1;
    }
    return rd;
}

int get_col_delta(int move_identifier){
    // returns change in col based on move identifier
    int rd = 2;
    if(move_identifier == 0 || move_identifier == 3 || move_identifier == 4 || move_identifier == 7){
        rd = 1;
    }
    if(move_identifier > 3){
        rd *= -1;
    }
    return rd;
}                

int pos_in_bounds(int cr, int cc, int r, int c){
    return cr < r && cc < c && cr >= 0 && cc >= 0;
}

int valid_moveid(int moveid, int crow, int ccol, int r, int c, char** state){
    // see if move in bounds first, then see if its not occupied

    int rpos = crow + get_row_delta(moveid);
    int cpos = ccol + get_col_delta(moveid);

    if(!pos_in_bounds(rpos, cpos, r, c)){
        return 0;
    }else{
        // position in bounds
        if(*(*(state+rpos)+cpos) != '\0'){
            return 0;
        }else{
            // position not occupied
            return 1;
        }
    }
}

struct state_info{
    int depth;
    char** board;

    // where this thread currently is
    int state_row;
    int state_col;
};

void free_state_info(struct state_info* state){
    for(int r=0;r<rows;++r){
        free(*(state->board + r));
    }
    free(state->board);
    free(state);
}

void* threaded_solve(void *args){
    // if we're in here we have a thread spawning responsibility...
    //
    // check for a solve
    //
    // determine valid moves
    //
    // set up thread launch args
    //
    // launch and thread
    //
    // catch and update

    // pointer cast
    struct state_info* thread_state = (struct state_info*) args;

    long int current_thread_number = next_thread_number;

    // compiler args in simulate function
    int quiet = 0;
#ifdef QUIET
    quiet = 1;
#endif

    int no_parallel = 0;
#ifdef NO_PARALLEL
    no_parallel = 1;
#endif

    // check for success
    if(thread_state->depth+1 == max_depth){
        // check if open or closed
        int closed_tour = 0;
        for(int i=0;i<EIGHT;++i){
            if(thread_state->state_row + get_row_delta(i) == start_row &&
                    thread_state->state_col+ get_col_delta(i) == start_col){ // closed
                closed_tour = 1;
                break;
            }
        }

        if(!quiet){
            if(closed_tour){
                printf("T%ld: Sonny found a full knight's tour; incremented total_closed_tours\n", current_thread_number);
            }else{
                printf("T%ld: Sonny found an open knight's tour; incremented total_open_tours\n", current_thread_number);
            }
        }        

        pthread_mutex_lock(&sync_lock);
        if(closed_tour){
            total_closed_tours++;
        }else{
            total_open_tours++;
        }
        pthread_mutex_unlock(&sync_lock);

        // TODO: free memory

        free_state_info(thread_state);

        return NULL;
        
    }

    // not done yet, find all valid moves - construct new thread state for each one
    // mark current position

    *(*(thread_state->board + thread_state->state_row) + thread_state->state_col) = 'x';

    pthread_t* subthread_log = calloc(EIGHT, sizeof(pthread_t));
    pthread_t* subthread_log_sim = calloc(EIGHT, sizeof(pthread_t));

    int subthread_count = 0;
    for(int moveid = 0;moveid < EIGHT;++moveid){
        if(valid_moveid(moveid, thread_state->state_row,
        thread_state->state_col, rows, cols, thread_state->board)){
            subthread_count++;
            *(subthread_log+moveid) = (pthread_t)1;
        }
    }

    int recursive_method = 0;

    if(subthread_count > 1){

        if(!quiet){
            if(current_thread_number == 0){
                printf("MAIN: %d possible moves after move #%d; creating %d child threads...\n", subthread_count, thread_state->depth+1, subthread_count);
            }else{
                printf("T%ld: %d possible moves after move #%d; creating %d child threads...\n", current_thread_number, subthread_count, thread_state->depth+1, subthread_count);
            }
        }

        for(int moveid = 0;moveid < EIGHT; ++moveid){

            if(*(subthread_log+moveid) == 0){
                // we flagged this move as invalid
                continue;
            }

            // construct its thread state

            struct state_info* spawned_thread_state = calloc(1, sizeof(struct state_info));

            // increment depth
            spawned_thread_state->depth = thread_state->depth + 1;

            // thread position, these change by move deltas
            spawned_thread_state->state_row = thread_state->state_row + get_row_delta(moveid);
            spawned_thread_state->state_col = thread_state->state_col + get_col_delta(moveid);

            // start building the new thread state board... alloc and copy
            spawned_thread_state->board = calloc(rows, sizeof(char*));
            for(int i=0;i<rows;++i){
                *(spawned_thread_state->board + i) = calloc(cols, sizeof(char));
                for(int j=0;j<cols;++j){
                    *(*(spawned_thread_state->board + i) + j) = *(*(thread_state->board + i) + j);
                }
            }

            next_thread_number++;

            // new thread state is constructed, create new thread
            *(subthread_log_sim+moveid) = (pthread_t)next_thread_number;

            int err = 1;
            while(err){
                err = pthread_create(subthread_log+moveid, NULL, threaded_solve, (void*)spawned_thread_state);
                if(err){
                    // error
                    printf("%ld threading failed\n", current_thread_number);
                    perror("error");
                }
            }
            

            if(no_parallel){
                // join thread immediately
                pthread_join(*(subthread_log + moveid), NULL);
                if(!quiet){
                    if(current_thread_number == 0){
                        printf("MAIN: T%ld joined\n", (long int)*(subthread_log_sim+moveid));
                    }else{
                        printf("T%ld: T%ld joined\n", current_thread_number, (long int)*(subthread_log_sim+moveid));
                    }
                }
            }
        }

    }else if(subthread_count == 1){
        // update current thread, recurse
        // dont need to make new thread state, just modify it

        thread_state->depth++;
        for(int i=0;i<EIGHT;++i){
            if(*(subthread_log + i) == 0){continue;}
            thread_state->state_row += get_row_delta(i);
            thread_state->state_col += get_col_delta(i);
            break;
        }

        // recurse
        free(subthread_log);
        free(subthread_log_sim);
        threaded_solve(thread_state);

        recursive_method = 1;
    }

    // done threading

    if(subthread_count==0){
        if(!quiet){
            if(thread_state->depth+1 > max_squares){
                printf("T%ld: Dead end at move #%d; updated max_squares\n", current_thread_number, thread_state->depth+1);
            }else{
                printf("T%ld: Dead end at move #%d\n", current_thread_number, thread_state->depth+1);
            }
        }
        if(thread_state->depth+1 > max_squares){
            max_squares = thread_state->depth + 1;
        }
    }else if(subthread_count > 1){
        if(!no_parallel){
            // join all threads here
            for(int i=0;i<EIGHT;++i){
                if(!(*(subthread_log + i))){continue;}
                pthread_join(*(subthread_log + i), NULL);
                if(!quiet){
                    if(current_thread_number == 0){
                        printf("MAIN: T%ld joined\n", (long int)*(subthread_log_sim+i));
                    }else{
                        printf("T%ld: T%ld joined\n", current_thread_number, (long int)*(subthread_log_sim+i));
                    }
                }
            }
        }
    }


    if(!recursive_method){
        free_state_info(thread_state);
        free(subthread_log);
        free(subthread_log_sim);
    }

    return NULL;

}

int simulate( int argc, char ** argv ){

    // see if all the arguments are there
    // these are the same as last time...
    
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
    rows = atoi(*(argv+1));
    cols = atoi(*(argv+2));
    max_depth = rows*cols;

    start_row = atoi(*(argv+3));
    start_col = atoi(*(argv+4));

    max_depth = rows * cols;

    if(!pos_in_bounds(start_row, start_col, rows, cols)){
        print_invalidusage();
        return 1;
    }

    // things we pass to the runtime...
    // the current board
    // where the newest location is
    // how many moves in we are

    next_thread_number = 0;

    // construct thread state struct pointer
    struct state_info* thread_state = calloc(1, sizeof(struct state_info));

    // construct thread state board, all '\0' for now
    thread_state->board = calloc(rows, sizeof(char*));
    for(int r=0;r<rows;++r){
        *(thread_state->board + r) = calloc(cols, sizeof(char));
    }

    thread_state->state_col = start_col;
    thread_state->state_row = start_row;

    printf("MAIN: Solving Sonny's knight's tour problem for a %dx%d board\n\
MAIN: Sonny starts at row %d and column %d (move #1)\n", rows, cols, start_row, start_col);

    threaded_solve(thread_state);

    if(total_closed_tours + total_open_tours == 0){
        if(max_squares == 1){
            printf("MAIN: Search complete; best solution(s) visited 1 square\n");
        }else{
            printf("MAIN: Search complete; best solution(s) visited %d squares out of %d\n", max_squares, max_depth);
        }
    }else{
        printf("MAIN: Search complete; found %d open tours and %d closed tours\n", total_open_tours, total_closed_tours);
    }

    pthread_mutex_destroy(&sync_lock);

    next_thread_number++;

    return 0;
}
