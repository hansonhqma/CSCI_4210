/* mutex-example.c */

/* multi-threaded example showing multiple threads adding to
 *  a shared set of global variables
 *
 * specifically, thread A sums 1 + 2 + ... + a, whereas
 *  thread B sums 1 + 2 + ... + b
 *
 * both threads A and B add to global_sum, which should
 *  then be 1 + 2 + ... + a + 1 + 2 + ... + b
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

/* shared global variables */
int global_a;    /* sum: 1 + 2 + ... + a */
int global_b;    /* sum: 1 + 2 + ... + b */
int global_sum;  /* global_a + global_b */

/* function prototypes for the thread functions */
void * whattodo_for_a( void * arg );
void * whattodo_for_b( void * arg );
void * noise( void * arg );

/* global mutex variables */
pthread_mutex_t mutex_on_global_sum = PTHREAD_MUTEX_INITIALIZER;

int main( int argc, char * argv[] )
{
  if ( argc != 3 )
  {
    fprintf( stderr, "ERROR: Invalid input(s)\n" );
    fprintf( stderr, "USAGE: a.out <a> <b>\n" );
    return EXIT_FAILURE;
  }

  int a = atoi( argv[1] );
  int b = atoi( argv[2] );

  int expected_sum_a = 0;
  for ( int i = 1 ; i <= a ; i++ ) expected_sum_a += i;

  int expected_sum_b = 0;
  for ( int i = 1 ; i <= b ; i++ ) expected_sum_b += i;

  int expected_global_sum = expected_sum_a + expected_sum_b;


  global_sum = global_a = global_b = 0;

  pthread_t tid1, tid2, tid3;
  int rc;

  rc = pthread_create( &tid1, NULL, whattodo_for_a, &a );
  if ( rc != 0 ) { fprintf( stderr, "pthread_create() failed (%d)\n", rc ); return EXIT_FAILURE; }

  rc = pthread_create( &tid2, NULL, whattodo_for_b, &b );
  if ( rc != 0 ) { fprintf( stderr, "pthread_create() failed (%d)\n", rc ); return EXIT_FAILURE; }

  /* TO DO: add a third call to pthread_create() and have that
   *         child thread call whattodo_for_b(), which then will
   *          require synchronization on global_b...
   */

  rc = pthread_create( &tid3, NULL, noise, NULL );


  rc = pthread_join( tid1, NULL );
  if ( rc != 0 ) { fprintf( stderr, "pthread_join() failed (%d)\n", rc ); return EXIT_FAILURE; }

  rc = pthread_join( tid2, NULL );
  if ( rc != 0 ) { fprintf( stderr, "pthread_join() failed (%d)\n", rc ); return EXIT_FAILURE; }

  /* no overlap in individual writes/updates to global_a */
  printf( "MAIN: global a is %d (expected %d)\n", global_a, expected_sum_a );
  
  /* no overlap in individual writes/updates to global_b */
  printf( "MAIN: global b is %d (expected %d)\n", global_b, expected_sum_b );

  /* potential overlap in individual writes/updates to global_sum */
  printf( "MAIN: global sum is %d (expected %d)\n", global_sum, expected_global_sum );

  return EXIT_SUCCESS;
}



void * whattodo_for_a( void * arg )
{
  int a = *(int *)arg;

  for ( int i = 1 ; i <= a ; i++ )
  {
    printf( "Thread A: adding %d to global_a\n", i );
    global_a += i;

    printf( "Thread A: adding %d to global_sum\n", i );
#if 0
usleep( 100 );  /* try running this code without this artificial delay */
#endif

    pthread_mutex_lock( &mutex_on_global_sum );
    {
      global_sum += i;
    }
    pthread_mutex_unlock( &mutex_on_global_sum );
  }

  return NULL;
}


void * whattodo_for_b( void * arg )
{
  int b = *(int *)arg;

  for ( int i = 1 ; i <= b ; i++ )
  {
    printf( "Thread B: adding %d to global_b\n", i );
    global_b += i;

    printf( "Thread B: adding %d to global_sum\n", i );

    pthread_mutex_lock( &mutex_on_global_sum );
    {
      global_sum += i;
    }
    pthread_mutex_unlock( &mutex_on_global_sum );
  }

  return NULL;
}



void * noise( void * arg )
{
  for ( int i = 1 ; i <= 1000 ; i++ )
  {
    usleep( 100 );
  }

  return NULL;
}







