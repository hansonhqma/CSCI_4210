#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include "utils.h"

#define WORD_LENGTH 5
#define ATTEMPTS 6
#define RESPONSE_PACKET_LENGTH 8
#define QUEUE_LIMIT 10

extern char** words;
extern int total_guesses;
extern int total_wins;
extern int total_losses;

pthread_mutex_t total_guess_lock;
pthread_mutex_t total_win_lock;
pthread_mutex_t total_loss_lock;
pthread_mutex_t handler_threads_lock;

char** all_words;
int WORD_COUNT = 0;
int SHUTDOWN_PROC = 0;
int ACTIVE_HANDLERS = 0;

int WORDS_PLAYED = 0;

struct timeval timeout;
int server_sock_fd;

// need actual wordle core tech
// single function that creates the 8 byte response packet
// remember that we need to convert outgoing data to network byte order (htons)
int core_checker(char* input, char* target_word, char* BUF);

// client handler - pthread_create calls this
void* handle_client(void* connected_sd);

// word in library checker
int valid_guess(char* guess);

// signal handler

void shutdown_handler();

// main callback
int wordle_server( int argc, char ** argv ){
    // validate input args
    setvbuf( stdout, NULL, _IONBF, 0 );

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
    
    for(char* cr = *(argv+4);*cr;++cr){
        WORD_COUNT *= 10;
        WORD_COUNT += *cr - '0';
    }

    // set rand seet
    srand(SEED);

    // populate words array
    FILE* words_fd = fopen(*(argv+3), "r");
    if(!words_fd){perror("ERROR: File opening failed");return EXIT_FAILURE;}

    // alloc and set words array
    all_words = calloc(WORD_COUNT+1, sizeof(char*));
    for(int i=0;i<WORD_COUNT;++i){
        *(all_words + i) = calloc(WORD_LENGTH + 1, sizeof(char));
        fscanf(words_fd, "%s", *(all_words+i));
    }
    fclose(words_fd);

    // print headers
    printf("MAIN: opened %s (%d words)\n", *(argv+3), WORD_COUNT);

    // set up tcp socket
    server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock_fd == -1){perror("ERROR: Socket creation failed");return EXIT_FAILURE;}

    if(setsockopt(server_sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1){perror("ERROR: setsockopt failed()");return EXIT_FAILURE;}

    // create internet socket address
    struct sockaddr_in server_name;
    server_name.sin_family = AF_INET;
    server_name.sin_addr.s_addr = htonl(INADDR_ANY);
    server_name.sin_port = htons(PORT_NUMBER);

    // bind address to socket
    // in this case we want to accept connection requests from any ip address
    if(bind(server_sock_fd, (struct sockaddr*)&server_name, sizeof(server_name)) == -1){
        perror("ERROR: bind() failed");
        close(server_sock_fd);
        return EXIT_FAILURE;
    }

    // set socket to listening state
    if(listen(server_sock_fd, QUEUE_LIMIT) == -1){
        perror("ERROR: listen() failed");
        return EXIT_FAILURE;
    }

    printf("MAIN: Wordle server listening on port {%d}\n", PORT_NUMBER);

    // set signal handlers
    signal(SIGTERM, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);
    signal(SIGUSR1, shutdown_handler);

    // set select timeoutt: 0 bytes in 0 blocks
    timeout.tv_sec = 2;
    timeout.tv_usec = 500;

#ifdef LOCAL
    printf("parent pid: %d\n", getpid());
#endif
    
    struct sockaddr_in client_name;
    socklen_t client_name_len = sizeof(client_name);
    while( 1 ){
        // block on accept, pass created socket fd

        int connected_fd = accept(server_sock_fd, (struct sockaddr*)&client_name, &client_name_len);
        if(connected_fd == -1){
            if(errno == 22){
                printf("MAIN: SIGUSR1 rcvd; Wordle server shutting down...\n Wordle server shutting down...\n");
                break;
            }
            perror("ERROR: accept() failed");
            return EXIT_FAILURE;
        }
        printf("MAIN: rcvd incoming connection request\n");

        // create child thread
        pthread_t th_id;
        int pth_result = pthread_create(&th_id, NULL, handle_client, (void*)&connected_fd);
        if(pth_result==-1){perror("ERROR: pthread_create() failed");return EXIT_FAILURE;}
        pth_result = pthread_detach(th_id);
        if(pth_result==-1){perror("ERROR: pthread_detach() failed");return EXIT_FAILURE;}

        pthread_mutex_lock(&handler_threads_lock);
        ACTIVE_HANDLERS++;
        pthread_mutex_unlock(&handler_threads_lock);
        
    }

    // go into waiting state for child threads to terminate
    while(ACTIVE_HANDLERS > 0){
        continue;
    }

    close(server_sock_fd);
    for ( char ** ptr = all_words ; *ptr ; ptr++ ) free( *ptr );
    free( all_words );

    return EXIT_SUCCESS;
} 

void* handle_client(void* sd_ptr){
    int connected_sd = *(int*)sd_ptr;

    unsigned long self_thid = pthread_self();


    // select target word
    int word_index = rand() % WORD_COUNT; // 0 ... WORD_COUNT - 1

    // add playing word to word bank

    *(words + WORDS_PLAYED) = calloc(WORD_LENGTH+1, sizeof(char));
    for(int i=0;i<WORD_LENGTH;++i){
        *(*(words+WORDS_PLAYED) + i) = toupper(*(*(all_words+word_index) + i));
    }
    WORDS_PLAYED++;
    words = realloc(words, sizeof(char*)*(WORDS_PLAYED+1));
    
    

    // create packet buffers
    char* incoming_buffer = calloc(WORD_LENGTH+1, sizeof(char));
    char* outgoing_buffer = calloc(RESPONSE_PACKET_LENGTH, sizeof(char));

    int game_state = 0;

    // set up fd set
    fd_set readfd;

    for(unsigned short turns_remaining = ATTEMPTS-1; turns_remaining >= 0; --turns_remaining){

        // block on recv and check for error
        int bytes_recv = 0;
        int client_quit = 0;

        printf("THREAD %lu: waiting for guess\n", self_thid);

        // receive packets until we have 5 bytes
        while(bytes_recv != WORD_LENGTH){
            // receive bytes until we have all 5
            // at this stage WORD_LENGTH - bytes_recv will never be 0

            if(SHUTDOWN_PROC){
                // in shutdown procedure
                free(incoming_buffer);
                free(outgoing_buffer);

                close(connected_sd);

                pthread_mutex_lock(&handler_threads_lock);
                ACTIVE_HANDLERS--;
                pthread_mutex_unlock(&handler_threads_lock);

                return NULL;
            }

            // see if our connected sd is ready to read from
            FD_ZERO(&readfd);
            FD_SET(connected_sd, &readfd);
            int ready = select(FD_SETSIZE, &readfd, NULL, NULL, &timeout);
            if(ready==0){continue;} // no activity yet...

            // if we've reached this point it means connected_sd has data in it
            // we can guarentee that this recv call will not block
            int sr_success = recv(connected_sd, incoming_buffer+bytes_recv, WORD_LENGTH - bytes_recv, 0);

            if(sr_success==-1){
                perror("ERROR: recv() failed");
                pthread_mutex_lock(&handler_threads_lock);
                ACTIVE_HANDLERS--;
                pthread_mutex_unlock(&handler_threads_lock);
                return NULL;
            }
            else if(sr_success==0){
                // client closed the socket
                printf("THREAD %lu: client gave up; closing TCP connection...\n", self_thid);
                client_quit = 1;
                break;
            }
            
            // mark how many bytes were recv
            bytes_recv += sr_success;

        }

        if(client_quit){
            break;
        }


        // basically at this point we should have a 5 byte packet put into 6 byte buffer
        printf("THREAD %lu: rcvd guess: %s\n", self_thid, incoming_buffer);

        // check if the guess is valid
        int vguess = valid_guess(incoming_buffer);
        if(vguess == 0){
            // not valid
            *outgoing_buffer = 'N';
            memset(outgoing_buffer + 3, '?', WORD_LENGTH);
            memset(incoming_buffer, '?', WORD_LENGTH);
            turns_remaining++;
            
        }else{
            *outgoing_buffer = 'Y';

            // run wordle...
            // we need to do this because the core checker logic looks at the buffer to determine word matching...
            memset(outgoing_buffer + 3, '\0', WORD_LENGTH);
            game_state = core_checker(incoming_buffer, *(all_words + word_index), outgoing_buffer+3);
            strncpy(incoming_buffer, outgoing_buffer+3, WORD_LENGTH);

            // increment total guesses
            pthread_mutex_lock(&total_guess_lock);
            total_guesses++;
            pthread_mutex_unlock(&total_guess_lock);
        }

        // convert turns remaining to network byte order, and write it
        unsigned short turns_remaining_nb = htons(turns_remaining);
        memcpy(outgoing_buffer + 1, &turns_remaining_nb, 2);


        printf("THREAD %lu: ", self_thid);
        if(vguess==0){
            printf("invalid guess; ");
        }
        printf("sending reply: %s ", incoming_buffer);
        if(turns_remaining == 1){
            printf("(1 guess left)\n");
        }else{
            printf("(%d guesses left)\n", turns_remaining);
        }

        // send response
        int sr_success = send(connected_sd, outgoing_buffer, RESPONSE_PACKET_LENGTH, 0);
        if(sr_success==-1){
            perror("ERROR: send() failed");
            pthread_mutex_lock(&handler_threads_lock);
            ACTIVE_HANDLERS--;
            pthread_mutex_unlock(&handler_threads_lock);
            return NULL;
        }

        if(game_state){
// win condition

            pthread_mutex_lock(&total_win_lock);
            total_wins++;
            pthread_mutex_unlock(&total_win_lock);

            break;
        }

        if(turns_remaining == 0){break;}
    }

    if(game_state == 0){
// loss condition
        pthread_mutex_lock(&total_loss_lock);
        total_losses++;
        pthread_mutex_unlock(&total_loss_lock);

    }

    for(int i=0;i<WORD_LENGTH;++i){
        *(incoming_buffer+i) = toupper(*(*(all_words + word_index) + i));
    }
    printf("THREAD %lu: game over; word was %s!\n", self_thid, incoming_buffer);
    

    // free memory and close socket
    free(incoming_buffer);
    free(outgoing_buffer);

    close(connected_sd);

    pthread_mutex_lock(&handler_threads_lock);
    ACTIVE_HANDLERS--;
    pthread_mutex_unlock(&handler_threads_lock);
    return NULL;
}

void shutdown_handler(){
    shutdown(server_sock_fd, 2);
    SHUTDOWN_PROC = 1;
}

int core_checker(char* input, char* target_word, char* BUF){
    // core wordle functionality
    // input should theoretically be 6 byte packet
    // target_word is 6 byte packet, last byte is '\0'
    // returns 1 on game win condition, 0 otherwise

    // assume BUF is pre-allocated packet of at least 6 bytes
    assert(BUF != NULL);

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

    int total_matches = 0;
    for(int i=0;i<WORD_LENGTH;++i){
        // find all matches first
        if(*(input + i) == *(target_word + i)){
            // match, put uppercase letter
            *(BUF + i) = toupper(*(input + i));
            // increment total matches
            total_matches++;
        }
    }

    for(int i=0;i<WORD_LENGTH;++i){
        if(*(BUF+i)){continue;}
        // second pass, assume matching places have been marked
        char guess_char = *(input + i);
        int map_idx = char_in_map(guess_char, map, WORD_LENGTH);
        if(!map_idx){
            // guess char not in map
            *(BUF + i) = '-';
        }else{
            // guess char in map and we've found all matches -> out of place char
            // what if all target chars have been used up? we have to check for this...
            if((*(map + map_idx) & 0xff) == 0){ // remember to bitmask leading byte...
                *(BUF + i) = '-';
            }
            else{
                *(BUF + i) = guess_char;
            }
        }
    }
    free(map);

    return total_matches == WORD_LENGTH;
}

int valid_guess(char* guess){
    for(int i=0;i<WORD_COUNT;++i){
        if(strcmp(guess, *(all_words + i)) == 0){
            return 1;
        }
    }
    return 0;
}
