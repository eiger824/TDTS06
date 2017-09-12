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

int parse_hostname(char* hostname, const char* buffer)
{
   char *substring = "Host:";
   char *match, *colon, *cr;
   unsigned i;

   match = strstr(buffer, substring);
   if (match != NULL)
   {
      colon = strchr(match, ':');
      if (!colon)
      {
         //No match, return
         return -1;
      }
      cr = strchr(colon, '\r');
      if (!cr)
      {
         //No match, return
         return -1;
      }
   }
   else
   {
      return -1;
   }
   //Copy the data to the hostname ptr
   for (i=colon-match+2; i<cr-match; ++i)
   {
      hostname[i-(colon-match+2)] = match[i];
   }
   //Zero-terminate
   hostname[cr-colon-1] = 0;
//Last extra-check
   if (hostname)
   {
      return 0;
   }
   else
   {
      return -1;
   }
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