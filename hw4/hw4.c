#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include "utils.h"

#define WORD_LENGTH 5
#define QUEUE_LIMIT 10

extern char** words;

// need actual wordle core tech
// single function that creates the 8 byte response packet
// remember that we need to convert outgoing data to network byte order (htons)

char* core_checker(char* input, char* target_word){
    // core wordle functionality
    // input should theoretically be 6 byte packet
    // target_word is 6 byte packet, last byte is '\0'

    char* ret = calloc(WORD_LENGTH+1, sizeof(char));

    unsigned short* map = calloc(WORD_LENGTH, sizeof(unsigned short));

    for(int i=0;i<WORD_LENGTH;++i){
        char target_char = *(target_word + i);
        int map_idx = char_in_map(target_char, map, WORD_LENGTH);
        if(!map_idx){
            // char not in map
            // assign and shift over 1 byte
            *(map+i) += (unsigned short)target_char;
            *(map+i) <<= 8;

            // increase count
            *(map+i) += 1;
        }else{
            // char was found somewhere in map
            *(map+map_idx) += 1;
        }
    }

    print_map(map, WORD_LENGTH);
    for(int i=0;i<WORD_LENGTH;++i){
        // find all matches first
        if(*(input + i) == *(target_word + i)){
            *(ret + i) = toupper(*(input + i));
            int map_idx = char_in_map(*(input + i), map, WORD_LENGTH);
            *(map + map_idx) -= 1;
        }
    }

    for(int i=0;i<WORD_LENGTH;++i){
        if(*(ret+i)){continue;}
        // second pass, assume matching places have been marked
        char guess_char = *(input + i);
        int map_idx = char_in_map(guess_char, map, WORD_LENGTH);
        if(!map_idx){
            // guess char not in map
            *(ret + i) = '-';
        }else{
            // guess char in map and we've found all matches -> out of place char
            // what if all target chars have been used up? we have to check for this...
            if((*(map + map_idx) & 0xff) == 0){ // remember to bitmax leading byte...
                *(ret + i) = '-';
            }
            else{
                *(ret + i) = guess_char;
            }
        }
    }
    


    free(map);
    return ret;
}


int wordle_server( int argc, char ** argv ){

    // validate input args

    if(argc != 5){
        fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: hw4.out <listener-port> <seed> <word-filename> <num-words>\n");
        return EXIT_FAILURE;
    }

    // assign server port, rand seed, and word count
    unsigned short PORT_NUMBER = 0;
    for(char* cr = *(argv+1);*cr;++cr){
        PORT_NUMBER *= 10;
        PORT_NUMBER += *cr - '0';
    }

    unsigned short SEED = 0;
    for(char* cr = *(argv+2);*cr;++cr){
        SEED *= 10;
        SEED += *cr - '0';
    }
    
    int WORD_COUNT = 0;
    for(char* cr = *(argv+4);*cr;++cr){
        WORD_COUNT *= 10;
        WORD_COUNT += *cr - '0';
    }

    // populate words array
    FILE* words_fd = fopen(*(argv+3), "r");
    if(!words_fd){perror("ERROR: File opening failed");return EXIT_FAILURE;}

    words = realloc(words, sizeof(char*)*(WORD_COUNT + 1));
    for(int i=0;i<WORD_COUNT;++i){
        *(words + i) = calloc(WORD_LENGTH + 1, sizeof(char));
        fscanf(words_fd, "%s", *(words+i));
    }
    fclose(words_fd);

    // print headers
    printf("MAIN: opened %s (%d words)\n", *(argv+3), WORD_COUNT);

    // set up tcp socket
    int listener_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(listener_socket == -1){perror("ERROR: Socket creation failed");return EXIT_FAILURE;}

    // create internet socket address
    struct sockaddr_in server_name;
    server_name.sin_family = AF_INET;
    server_name.sin_addr.s_addr = htonl(INADDR_ANY);
    server_name.sin_port = htons(PORT_NUMBER);

    // bind name to socket
    if(bind(listener_socket, (struct sockaddr*)&server_name, sizeof(server_name)) == -1){
        perror("ERROR: bind() failed");
        return EXIT_FAILURE;
    }

    // set accept and queue limit
    if(listen(listener_socket, QUEUE_LIMIT) == -1){
        perror("ERROR: listen() failed");
        return EXIT_FAILURE;
    }
    
    printf("MAIN: Wordle server listening on port %d\n", PORT_NUMBER);

    // TODO: everything past this point is just for testing

    // set up client socket addr
    struct sockaddr_in client_name;
    socklen_t client_name_len = sizeof(client_name);

    // blocking call
    printf("blocking on accept...\n");
    int sd = accept(listener_socket, (struct sockaddr*)&client_name, &client_name_len);
    if(sd==-1){
        perror("ERROR: accept() failed");
        return EXIT_FAILURE;
    }
    printf("client accepted\n");

    printf("blocking on recv");

    char* recv_buffer = calloc(WORD_LENGTH+1, sizeof(char));
    
    int n = recv(sd, recv_buffer, WORD_LENGTH, 0);
    if(n==-1){
        perror("ERROR; recv() failed");
        return EXIT_FAILURE;
    }

    char* cc = core_checker(recv_buffer, "wears");
    printf("%s\n", cc);




    return EXIT_SUCCESS;
} 