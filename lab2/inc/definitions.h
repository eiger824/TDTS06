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

bool check_if_http_get(const char* buffer){
	return buffer[0] == 'G';
}

int parse_hostname(char* hostname, const char* buffer)
{
	char *substring = "Host:";
	char *match, *colon, *cr;
	unsigned i;

	// Check if http get
	if(!check_if_http_get(buffer)){
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

/*
Function: print_data
Param: buffer to print
Returns: nothing
*/

void print_data(char* buffer, unsigned hex)
{
	unsigned i;
	printf("====================Buffer====================\n");
	if (hex)
	{
		hexify(buffer);
	}
	else
	{
		for (i=0; i<strlen(buffer); ++i)
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
#endif /*DEFINITIONS_H_*/
