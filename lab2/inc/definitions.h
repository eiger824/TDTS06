#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#include <pthread.h>

/*
  Maximum number of bytes to write/read from the socket
 */
#define MAX_BUFFER_LENGTH 10000

/*
  Version of the current program
 */

#define VERSION "0.1"

/*
  Maximum number of simultaneous requests that our proxy can handle
 */
unsigned MAX_SIM_REQUESTS = 10;

/*
  Array with the created threads
 */
pthread_t *threads;

/*
 Default port number to be used 
 */
int portno = 3422;

/*
  Function: get_size(thread_t p*)
  In-param: Pointer to array of threads
  Returns: length of the array
 */
static unsigned get_size(pthread_t *p)
{
  unsigned c = 0;
  if (!p) return c;
  while (*p)
    {
      ++c;
      ++p;
    }
  return 0;
}

bool parse_hostname(char* hostname, const char* buffer)
{
  unsigned j,i;
  for ( i=0; i < strlen(buffer)-5; ++i)
    {
      if (buffer[i] == 'H' &&
	  buffer[i+1] == 'o' &&
	  buffer[i+2] == 's' &&
	  buffer[i+3] == 't' &&
	  buffer[i+4] == ':')
	{
	  j=i+6;
	  do {
	    hostname[j-(i+6)] = buffer[j];
	    ++j;
	  } while (buffer[j] != '\r');
	  break;
	}
    }
  hostname[j] = 0;
  if (hostname) return true;
  else return false;
}

void hexify(char* buffer)
{
  unsigned i;
  
  for (i=0; i<strlen(buffer)-1; ++i)
    {
      if (buffer[i] == 0xd && buffer[i+1] == 0xa)
	{
	  printf(" (CR + NL)\n");
	  ++i;
	}
      else
	{
	  printf("%x%s", buffer[i],(i+1==strlen(buffer))?"":"-");
	}
    }
}

#endif /*DEFINITIONS_H_*/
