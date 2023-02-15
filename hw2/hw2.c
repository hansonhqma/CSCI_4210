/*
 * CSCI 4210 homework 2
 * solves knights tour problem with multiprocessing
 * hanson.hq.ma@gmail.com
 *
 */


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define EIGHT 8

void print_state(char** state, int r, int c, int crow, int ccol){
    for(int i=0;i<r;++i){
        for(int j=0;j<c;++j){
            if(i==crow && j==ccol){
                printf(" X");
            }else{
                if(*(*(state+i)+j)=='\0'){
                    printf(" -");
                }else{
                    printf(" %c", *(*(state+i)+j));
                }
            }
        }
    }
    printf("\n");
}

void print_invalidusage(){
    fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: hw2.out <m> <n> <r> <c>\n");
}

int pos_in_bounds(int cr, int cc, int r, int c){
    return cr < r && cc < c && cr >= 0 && cc >= 0;
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
    int rd = 2;
    if(move_identifier == 0 || move_identifier == 3 || move_identifier == 4 || move_identifier == 7){
        rd = 1;
    }
    if(move_identifier > 3){
        rd *= -1;
    }
    return rd;
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

int main(int argc, char** argv){

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

    // set up compiler flags
    int quiet = 1;
#ifndef QUIET
    quiet = 0;
#endif

    int no_parallel = 1;
#ifndef NO_PARALLEL
    no_parallel = 0;
#endif

    setvbuf( stdout, NULL, _IONBF, 0  );

    int rows = atoi(*(argv+1));
    int cols = atoi(*(argv+2));
    int start_row = atoi(*(argv+3));
    int start_col = atoi(*(argv+4));

    if(!pos_in_bounds(start_row, start_col, rows, cols)){
        print_invalidusage();
        return 1;
    }


    printf(" Solving Sonny's knight's tour problem for a %dx%d board\n", rows, cols);
    printf(" Sonny starts at row %d and column %d (move #1)\n", start_row, start_col);

    // set up pipe
    int* fd = calloc(2, sizeof(int));
    if(pipe(fd)==-1){
        exit(errno);
    }

    // all checks are done
    // runtime variables
    int current_row = start_row;
    int current_col = start_col;

    int responsibility = 1;
    int* subprocess_log = calloc(EIGHT, sizeof(int));

    // create state
    // '\0': have not moved here

    char** state = calloc(rows, sizeof(char*));
    for(int r=0;r<rows;++r){
        *(state+r) = calloc(cols, sizeof(char));
    }

    int depth = 0;
    int max_search = 0;
    int TOP_LEVEL_PID = getpid();

    int subprocess_count;
    while(responsibility){
        // while i have the responsibility of spawning processes...

        if(depth+1 == rows*cols){
            // if there are as many occupied spots as there are moves... i must be done

            // #TODO: check if open or closed
            int closed_tour = 0;
            for(int i=0;i<EIGHT;++i){
                if(current_row + get_row_delta(i) == start_row &&
                        current_col + get_col_delta(i) == start_col){ // closed
                    closed_tour = 1;
                    break;
                }
            }

            if(!quiet){
                if(closed_tour){
                    printf("PID %d: Sonny found a full knight's tour; notifying top-level parent\n", getpid());
                }else{
                    printf("PID %d: Sonny found an open knight's tour; notifying top-level parent\n", getpid());
                }
            }

            // write to pipe
            char* write_buffer = calloc(1, 1);
            if(closed_tour){
                *write_buffer = 'c';
            }else{
                *write_buffer = 'o';
            }

            close(*fd); // close read end
            write(*(fd+1), write_buffer, 1);

            // free
            for(int i=0;i<rows;++i){
                free(*(state+i));
            }
            free(state);
            state = NULL;
            free(subprocess_log);
            subprocess_log = NULL;
            free(fd);
            free(write_buffer);

            exit(depth);
        }

        // take advantage of copy-on-write mechanisms
        *(*(state+current_row)+current_col) = 'x';

        // RRAAHHHHHHHHKJNDGLKSHJBGLJKHSBFKJHBKFJHBSKDJGHBSKJHGBK
        // 😡🤬😡🤬😡🤬😡🤬😡🤬😡🤬😡🤬😡🤬😡🤬

        // need to find how many legal moves there are first...
        subprocess_count = 0;
        for(int i=0;i<EIGHT;++i){
            if(valid_moveid(i, current_row, current_col, rows, cols, state)){
                subprocess_count++;
                *(subprocess_log+i) = 1; // just a signal
            }else{
                *(subprocess_log+i) = 0;
            }
        }
        
        depth++;

        if(subprocess_count > 1){
            if(!quiet){
                printf("PID %d: %d possible moves after move #%d; creating %d child processes...\n", getpid(), subprocess_count, depth, subprocess_count);
            }
            for(int move_id = 0;move_id<EIGHT;++move_id){

                // find moves in clockwise order
                // how can we do this while minimizing memory allocation...?
                // for each move identifier
                // generate deltas and check for validity:
                //      - is it in bounds
                //      - is it a free square
                //      only after those two are true can we spawn processes

                if(*(subprocess_log+move_id) == 0){
                    // this move_id is not valid
                    continue;
                }

                int row_delta = get_row_delta(move_id);
                int col_delta = get_col_delta(move_id);
                // we'll change these back after the child process spawns...
                current_row += row_delta;
                current_col += col_delta;

                // we can spawn a process to move there
                int spawn_pid = fork();
                if(!spawn_pid){
                    // in child process
                    //printf("child process %d spawned at depth %d\n", getpid(), depth);
                    responsibility = 1;
                    break;

                }else if(spawn_pid==-1){
                    // fork failed, terminate with errno
                    perror("fork failed");
                    exit(errno);

                }else{
                    if(no_parallel){
                        int status;
                        waitpid(spawn_pid, &status, 0);
                        if(WIFEXITED(status)){
                            int exit_status = WEXITSTATUS(status);
                            if(exit_status > max_search){
                                max_search = exit_status;
                            }
                        }else{
                            perror("child did not terminate normally");
                        }
                    }
                    // in parent process
                    responsibility = 0;
                    *(subprocess_log+move_id) = spawn_pid; // record child pid

                    // change current position back
                    current_row -= row_delta;
                    current_col -= col_delta;
                }
            }
        
        }else if(subprocess_count == 1){
            // no need to fork
            for(int moveid=0;moveid<EIGHT;++moveid){
                if(*(subprocess_log+moveid) == 0){
                    continue;
                }
                current_row += get_row_delta(moveid);
                current_col += get_col_delta(moveid);
            }
        }else{
            // no sub processes, exit responsibility loop
            responsibility = 0;

        }
    }


    // done spawning children


    if(!subprocess_count){
        if(!quiet){
            printf("PID %d: Dead end at move #%d\n", getpid(), depth);
        }
        max_search = depth;
    }else{
        if(!no_parallel){
            // parallelized process catching, wait for children
            int status;
            for(int i=0;i<EIGHT;++i){

                if(!(*(subprocess_log+i))){continue;}

                waitpid(*(subprocess_log+i), &status, 0);

                if(WIFEXITED(status)){
                    if(WEXITSTATUS(status) > max_search){
                        max_search = WEXITSTATUS(status);
                    }
                }else{
                    perror("child did not terminate normally");
                }
            }
        }
    }

    if(TOP_LEVEL_PID == getpid()){
        // at this point all solutions should've been written to fd...
        int open = 0;
        int closed = 0;

        close(*(fd+1)); // close write end
        char* read_buffer = calloc(1, 1);
        while(read(*fd, read_buffer, 1)){
            if(*read_buffer == 'c'){closed++;}
            else{open++;}
        }
        free(read_buffer);

        if(open+closed==0){
            // no solutions were found
            printf(" Search complete; best solution(s) visited %d squares out of %d\n", max_search, rows*cols);
        }else{
            printf(" Search complete; found %d open tours and %d closed tours", open, closed);
        }
        

    }

    // free up memory
    free(fd);
    for(int i=0;i<rows;++i){
        free(*(state+i));
    }
    free(state);
    state = NULL;
    free(subprocess_log);
    subprocess_log = NULL;


    return max_search;
}
