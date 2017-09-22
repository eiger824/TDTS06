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

#define HIGH 1
#define LOW 0
#define DEAD -1

/*
  Maximum number of simultaneous requests that our proxy can handle
*/
unsigned MAX_SIM_REQUESTS = 100;

/*
  Array with the created threads
*/
pthread_t *threads;

/*
  Default port number to be used
*/
int portno = 3422;

typedef enum {
   IDLE,
   HALF_CONNECTION,
   FULL_CONNECTION,
   BUFFERING
} STATE;


/*
  Thread data structure
  thread_id: a numerical ID assigned to the working thread
  priority: priority of that thread (to be determined)	
*/

struct thread_data
{
   int thread_id;
   short priority;
   int cli_sock_fd;
};

bool check_if_http_get(const char* buffer)
{
   return buffer[0] == 'G';
}

bool check_if_http_connect(const char* buffer)
{
   return buffer[0] == 'C';
}

bool check_if_http_post(const char* buffer)
{
   return buffer[0] == 'P';
}

int parse_hostname(char* hostname, const char* buffer)
{
   char *substring = "Host:";
   char *match, *colon, *cr;
   unsigned i;
   char first_line[500];
   for (i=0; buffer[i] != '\r'; ++i)
   {
      first_line[i] = buffer[i];
   }
   first_line[i] = '\0';

   // Check if http get
   if(!check_if_http_get(first_line) && !check_if_http_connect(first_line))
   {
      return -2;
   }

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
   hostname[cr-colon-2] = 0;
   //Last extra-check
   if (hostname)
   {
      if (check_if_http_get(buffer))
         return 1;
      else if (check_if_http_connect(buffer))
         return 2;
      else
         return 3;
   }
   else
   {
      return -1;
   }
}



void hexify(char* buffer, int n)
{
   int i;
   for (i=0; i<n-1; ++i)
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

/*
  Function: print_data
  Param: buffer to print
  Returns: nothing
*/

void print_data(char* buffer, int n, unsigned hex)
{
   int i;
   printf("====================Buffer====================\n");
   if (hex)
   {
      hexify(buffer, n);
   }
   else
   {
      for (i=0; i<n; ++i)
      {
         if (32 <= buffer[i]/* && buffer[i] <= 127*/) //signed char, always < 127
         {
            printf("%c", buffer[i]);
         }
         else if (buffer[i] == 9 || buffer[i] == 10 || buffer[i] == 13)
         {
            if (buffer[i] == 9) printf("\\t");
            else if (buffer[i] == 10) printf("\\n\n");
            else if (buffer[i] == 13) printf("\\r");
         }
         else
         {
            printf("%#x", buffer[i]);
         }
      }
   }
   printf("==============================================\n");
}

/* return: -2 if not http header
           -1 if incomplete
            0 if success
  */
int extract_http_info(const char* msg,
                      char* hostname,
                      int* method_type,
                      int *content_length,
                      int *content_type,
                      int *http_header_length)
{

   char *token = "HTTP/";
   char *match;
   char first_line[500];
   unsigned i, j;
   int ret, index;


   // Safety check. HTTPS messages might be super weird
   for (i=0; (msg[i] != '\r' && msg[i] != '\0' && msg[i] != '\n'); ++i)
   {
      first_line[i] = msg[i];
   }
   first_line[i] = '\0';

   // Check if http header
   match = strstr(first_line, token);

   if (match != NULL) //HTTP header
   {
      // Check if the request is complete
      char *header_end = "\r\n\r\n";
      match = strstr(msg, header_end);


      if (match == NULL)
      {
        return -1;
      }
      else
      {
        *http_header_length = match - msg + 4;
      }
      // Look for hostname
      if ((ret = parse_hostname(hostname, msg)) < 0)
      {
         //http response (hostname NOT updated)
         char *length = "Content-Length: ";
         char *type = "Content-Type: ";

         //Content-length
         match = strstr(msg, length);
         if (match != NULL) //found a match
         {
            index = match - msg + strlen(length);
            char len[20];
            for (j=index; msg[j] != '\r'; ++j)
            {
               len[j-index] = msg[j];
            }
            len[j-index] = '\0';
            *content_length = atoi(len);
         }
         else
         {
            *content_length = -1;
            //return -1;
         }

         //Content-type
         match = strstr(msg, type);
         if (match != NULL)
         {
            index = match - msg + strlen(type);
            char typ[50];
            for (j=index; msg[j] != '\r'; ++j)
            {
               typ[j-index] = msg[j];
            }
            typ[j-index] = '\0';
            if (strstr(typ, "text") != NULL)
            {
               *content_type = 1;
            }
            else
            {
               *content_type = 0;
            }
         }
         else
         {
            *content_type = -1;
            //return -1;
         }

         return 0; //success
      }
      else
      {
         //update method-type (hostname is already updated)
         *method_type = ret;
         *content_length = -1;
         *content_type = -1;
         return 0; //request
      }
   }
   else //Not found, not HTTP header
   {
      *content_length = -1;
      *content_type = -1;
      return -2;
   }
}
#endif /*DEFINITIONS_H_*/
