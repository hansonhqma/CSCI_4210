



#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

void print_invalidusage(){
    perror("ERROR: Invalid argument(s)\n");
    perror("USAGE: hw2.out <m> <n> <r> <c>\n");
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
    int start_rows = atoi(argv[3]);
    int start_cols = atoi(argv[4]);
    
    // set up pipe
    int fd[2];
    if(pipe(fd)==-1){
        return -1;
    }

    


    return 0;
}
