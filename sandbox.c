#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <float.h>

#define MAX_BUFFER_LEN 256

typedef struct{
    FILE* data;
    int status;
} resource; // simulate a resource


int main(){

    char buffer[MAX_BUFFER_LEN];

    scanf("%[0-9]", buffer);

    printf("%s\n", buffer);



    return 0;
}

