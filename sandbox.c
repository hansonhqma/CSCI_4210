#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define MAX_BUFFER_LEN 256

int main(int argc, char** argv){


    char mychar = '5';

    int myint = mychar-'0';

    printf("%d\n", myint);
    
    return 0;
}

