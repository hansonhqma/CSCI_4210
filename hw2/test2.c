/*
 * =====================================================================================
 *
 *       Filename:  test2.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/10/2023 01:54:59 PM
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

int main(){

    
    int cpid = fork();
    
    if(!cpid){
        // child
        printf("child:\t%d, pgid: %d\n", getpid(), getpgid(getpid()));


        if(!fork()){
            // second level child
            printf("child:\t%d, pgid: %d\n", getpid(), getpgid(getpid()));
            sleep(1);
            exit(0);
            
        }else{
            printf("parent:\t%d\n", getpid());
            int status;
            waitpid(-getpid(), &status, 0); // should catch all
        }

        sleep(1);
        
        exit(0);

        
    }else{
        // parent
        printf("parent:\t%d\n", getpid());

        int status;
        waitpid(-getpid(), &status, 0); // should catch all

    }


    exit(0);
}
