#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

void print_msg(void *ptr){
    char *message = (char*) ptr;

    printf("%s\n", message);
}


int main(){

    pthread_t thread1_identifier, thread2_identifier;

    char *ms1 = "thread1 msg";
    char *ms2 = "thread2 msg";

    // this creates and runs threads
    int thread1_result = pthread_create(&thread1_identifier, NULL, print_msg, (void*)ms1);
    int thread2_result = pthread_create(&thread2_identifier, NULL, print_msg, (void*)ms2);


    return 0;
}
