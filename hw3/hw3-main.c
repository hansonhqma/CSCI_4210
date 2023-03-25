/* hw3-main.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

long next_thread_number;
int max_squares;
int total_open_tours;
int total_closed_tours;

/* write the simulate() function and place all of your code in hw3.c */
int simulate( int argc, char ** argv );

int main( int argc, char ** argv )
{
  next_thread_number = 1;  /* or other positive long value */
  max_squares = total_open_tours = total_closed_tours = 0;
  int rc = simulate( argc, argv );

  /* on Submitty, there will be more code here that validates
   *  the global variables at the end of your code...
   */

  return rc;
}
