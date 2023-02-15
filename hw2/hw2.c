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

void print_prompt(){
    printf("PID %d: ", getpid());
}

void print_invalidusage(){
    perror("ERROR: Invalid argument(s)\n");
    perror("USAGE: hw2.out <m> <n> <r> <c>\n");
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

    setvbuf( stdout, NULL, _IONBF, 0  );

    int rows = atoi(*(argv+1));
    int cols = atoi(*(argv+2));
    int start_row = atoi(*(argv+3));
    int start_col = atoi(*(argv+4));

    if(!pos_in_bounds(start_row, start_col, rows, cols)){
        print_invalidusage();
        return 1;
    }

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
    int TOP_LEVEL_PID = getpid();

    int subprocess_count;
    while(responsibility){
        // while i have the responsibility of spawning processes...
        subprocess_count = 0;

        if(depth+1 == rows*cols){
            // if there are as many occupied spots as there are moves... i must be done
            //printf("process %d at depth %d:\n", getpid(), depth);
            print_prompt();
            printf("Sonny found a full knight's tour; notifying top-level parent\n");

            // #TODO: check if open or closed
            int closed_tour = 0;
            for(int i=0;i<EIGHT;++i){
                if(current_row + get_row_delta(i) == start_row &&
                        current_col + get_col_delta(i) == start_col){ // closed
                    closed_tour = 1;
                    break;
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
        char token;
        if(depth>9){
            token = 'a' + (depth-10);
        }else{
            token = '0' + depth;
        }
        *(*(state+current_row)+current_col) = token;
        for(int i=0;i<EIGHT;++i){
           *(subprocess_log+i)= 0;
        }

        depth++;

        for(int move_id = 0;move_id<EIGHT;++move_id){

            // find moves in clockwise order // how can we do this while minimizing memory allocation?
            // for each move identifier
            // generate deltas and check for validity:
            //      - is it in bounds
            //      - is it a free square
            //      only after those two are true can we spawn processes
            int row_delta = get_row_delta(move_id);
            int col_delta = get_col_delta(move_id);
            
            // we'll change these back after the child process spawns...
            current_row += row_delta;
            current_col += col_delta;
            
            if(!pos_in_bounds(current_row, current_col, rows, cols)){

                // position out of bounds
                current_row -= row_delta;
                current_col -= col_delta;
                responsibility = 0;
                continue;
            }
            if(*(*(state + current_row) + current_col) != '\0'){

                // position is occupied 
                current_row -= row_delta;
                current_col -= col_delta;
                responsibility = 0;
                continue;
            }

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
                // in parent process
                subprocess_count++;
                
                responsibility = 0;
                *(subprocess_log+move_id) = spawn_pid; // record child pid

                // change current position back
                current_row -= row_delta;
                current_col -= col_delta;

            }
        }
    }



    int max_search = 0;

    if(!subprocess_count){
        // dead-end
        print_prompt();
        printf("Dead end at move #%d\n", depth);
        max_search = depth;
    }else{
        // i spawned some children...
        if(TOP_LEVEL_PID == getpid()){
            print_prompt();
            printf("%d possible moves after move #%d; creating %d child processes...\n", subprocess_count, depth, subprocess_count);
        }

        // wait for child processes
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
        printf("%d open, %d closed\n", open, closed);
        free(read_buffer);

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
