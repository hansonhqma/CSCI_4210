#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <float.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <wait.h>

#define MAX_BUFFER_LEN 256


int main(){

    unsigned short bits = 99;

    bits <<= 8; // shift over one byte

    bits += 2;

    bits >>= 8;

    char tgt = 'c';

    int eq = bits == tgt;

    printf("%u\n", bits);
    printf("%c\n", bits);
    printf("%d\n", eq);

    


    return 0;
}

