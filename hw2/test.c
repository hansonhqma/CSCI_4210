/*
 * =====================================================================================
 *
 *       Filename:  test.c
 *
 *    Description:  fun with pipes
 *
 *        Version:  1.0
 *        Created:  02/03/2023 03:40:44 PM
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
#include <sys/wait.h>
#include <sys/types.h>

#define target_depth 1


int main(){

    // communication pipe
    int fd[2];

    if(pipe(fd)==-1){
        return -1;
    }

    /* some notes
     *
     * goal here: create tree of process with depth = 2 and breadth = 3
     *
     * each parents process only needs to spawn children once... and collect them
     * process runtime:
     * - see if it even needs to spawn children... how?
     * - if it does:
     *   - do that, wait and catch them, exit
     * - if it doesn't:
     *   - tell top level process something cool and exit
     *
     *
     * but how to processes know if they need to spawn children?
     * we can tell them using copied stack variables?
     * but how tf does the control loop work..
     *
     * throw that bitch in a while loop
     * - newly created child processes have to execute again
     * - old parent processes break out and wait
     *
     */

    int depth = 0;

    int responsibility = 1;

    printf("top level process %d\n", getpid());

    int* child_log;

    while(responsibility){
        if(depth){
            // if we're not the top-level process... we have a copied live pointer. need to free
            free(child_log);
        }

        printf("process %d at start of control loop\n", getpid());
        if(depth==target_depth){

            // leaf node condition

            sleep(5);
            printf("leaf node %d exiting, fd: %d, %d\n", getpid(), fd[0], fd[1]);
            exit(69);
        }

        // at this point we're not a leaf node... so we need to create children
        // and set a responsibility level:
        //  - 1 means i have a responsibility to spawn children
        //  - 0 means i just spawned, and i need to wait for my children

        // before we spawn individual children we need to set the depth stack variable
        depth++;

        // need to make sure im not a child proces
        child_log = calloc(3, sizeof(int));
        
        for(int i=0;i<3;++i){
            printf("parent process %d spawning %dth children at depth %d\n", getpid(), i, depth);

            // spawn child and keep track of pids
            sleep(1);
            int child_pid = fork();

            if(!child_pid){
                // in child process, this guy has to run the loop again
                // newly spawned

                responsibility = 1;
                printf("\tchild process %d spawned at depth %d with pid %d\n", i, depth, getpid());

                break;

            }else{
                // in parent process that just spawned children,
                // need to make sure from this point on the loop exits and waits
                child_log[i] = child_pid; // record children spawning
                responsibility = 0;
                
            }
        }

    }

    int* status = calloc(1, sizeof(int));
    for(int i=0;i<3;++i){
        waitpid(child_log[i], status, 0);
        if(WIFEXITED(*status)){
            printf("parent %d caught child %d with exit status %d\n", getpid(), child_log[i], WEXITSTATUS(*status));
        }
        else{
            printf("child did not terminate normally\n");
        }
    }


    free(child_log);
    free(status);

    return 0;
}
