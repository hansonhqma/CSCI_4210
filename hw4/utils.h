#include <stdio.h>

int char_in_map(char c, unsigned short* map, int mapsize){
    // checks if c is any of the first bytes
    for(int i=0;i<mapsize;++i){
        unsigned short char_partition = *(map+i) >> 8;
        if(c == char_partition){
            return i;
        }
    }

    return 0;
}

void print_map(unsigned short* map, int mapsize){
    for(int i=0;i<mapsize;++i){
        printf("%c: %u\n", *(map+i)>>8, *(map+i) & 0xff);
    }
}