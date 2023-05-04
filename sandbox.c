#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <float.h>
#include <limits.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <wait.h>

#include <arpa/inet.h>

#define MAX_BUFFER_LEN 256


int main(){

    fprintf(stdout, "writing to stdout\n");

    fprintf(stderr, "writing to stderr\n");


    return 0;
}

