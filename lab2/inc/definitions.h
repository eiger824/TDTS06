#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#include <pthread.h>

/*
  Maximum number of bytes to write/read from the socket
*/
#define MAX_BUFFER_LENGTH 100000

/*
  Version of the current program
*/
#define VERSION "0.1"


/** Description: Priorities to use when creating threads
 */
#define HIGH 1
#define LOW 0
#define DEAD -1


/** Name: MAX_SIM_REQUESTS
    Description: Maximum number of simultaneous requests that our
                 proxy can handle. Defaults to 100.
*/
unsigned MAX_SIM_REQUESTS = 100;


/** Name:        threads
    Description: Array with the created threads
*/
pthread_t *threads;


/** Name:        portno
    Description: Default port number to be used
*/
int portno = 3422;


/** Name:        STATE
    Description: The existing states in the thread function handler
 */
typedef enum {
   IDLE,
   HALF_CONNECTION,
   FULL_CONNECTION,
   BUFFERING
} STATE;


/** Name:               thread_data
    Description:        The thread data structure to pass on to the
                        thread function handler
    @field thread_id:   A numerical ID assigned to the working thread
    @field priority:    Priority of that thread (to be determined)
    @field cli_sock_fd: The socket file descriptor associated with the
                        incoming connection from the client
*/
struct thread_data
{
   int thread_id;
   short priority;
   int cli_sock_fd;
};

#endif /*DEFINITIONS_H_*/
