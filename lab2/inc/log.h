#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define ERROR 0
#define INFO 1

struct timeval time_before, time_after, time_result;

void print(unsigned level, const char* msg)
{
   //update the time struct
   gettimeofday(&time_after, NULL);
   timersub(&time_before, &time_after, &time_result);

   long int usecs = (long int)time_result.tv_usec;
   time_t timer;
   char time_str[26];
   char *buffer = (char*) malloc(100000);
   struct tm* tm_info;

   time(&timer);
   tm_info = localtime(&timer);

   strftime(time_str, 26, "%Y-%m-%d %H:%M:%S", tm_info);
   strcpy(buffer, "[");
   strcat(buffer, time_str);
   strcat(buffer, "");
   strcat(buffer, "] ");
   strcat(buffer, msg);
   strcat(buffer, "\n");

   buffer[strlen(buffer)] = '\0';
   
   if (!level)
      fprintf(stderr, buffer);
   else
      printf(buffer);
}

#endif /*LOG_H_*/
