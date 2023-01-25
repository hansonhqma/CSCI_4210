#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <float.h>
#include <limits.h>

#define MAX_BUFFER_LEN 256

typedef struct{
    FILE* data;
    int status;
} resource; // simulate a resource


int main(){

    unsigned int buffer;

    printf("%u\n", UINT_MAX);
    scanf("%u", &buffer);


    printf("%u\n", buffer);



    return 0;
}

