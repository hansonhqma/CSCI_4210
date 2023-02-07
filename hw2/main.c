/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  solves knights tour problem with multiprocessing 
 *
 *        Version:  1.0
 *        Created:  02/03/2023 05:09:35 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
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
    /* some notes
     * closed tour = finishes one move away from start
     * open tour = not a closed tour
     *
     * when child process finds a solution, tells parent process
     *
     * each intermediate parent process needs to be able to give knowledge to its
     * child processes in the form of copied stack variables
     *
     */

    


    return 0;
}
