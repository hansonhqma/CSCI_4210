#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <float.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>

#define MAX_BUFFER_LEN 256


int main(){

    int fd[2];

    close(0);

    pipe(fd);

    printf("%d\n", getpid());

    printf("%d, %d\n", fd[0], fd[1]);



    return 0;
}

