#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define ERROR 0
#define INFO 1

#define log_info(...)      print(INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...)     print(ERROR, __FILE__, __LINE__, __VA_ARGS__)

struct timeval time_before, time_after, time_result;

void print(unsigned level,       /* level: ERROR (stderr) / INFO (stdout) */
           const char* filename, /* filename: The current file where the logging line is */
           unsigned line,        /* line: The current line where the logging line is */
           const char* msg, ...) /* msg: Formated message with following (variadic) args */
{
   //update the time struct
   gettimeofday(&time_after, NULL);
   timersub(&time_before, &time_after, &time_result);

   va_list args;
   
   long int usecs = (long int)time_result.tv_usec;
   time_t timer;
   struct tm* tm_info;

   time(&timer);
   tm_info = localtime(&timer);

   char time_str[50];
   time_str[strftime(time_str, 26, "%Y-%m-%d %H:%M:%S", tm_info)] = '\0';

   char usecs_str[20];
   usecs_str[sprintf(usecs_str, ".%06ld", usecs)] = '\0';

   if (!level)
      fprintf(stderr, "[%s.%s] (%s:%d) ", time_str, usecs_str, filename, line);
   else
      printf("[%s%s] (%s:%d) ", time_str, usecs_str, filename, line);
   
   va_start(args, msg);
   if (!level)
   {
      vfprintf(stderr, msg, args);
      fprintf(stderr, "\n");
   }
   else
   {
      vfprintf(stdout, msg, args);
      printf("\n");
   }
   va_end(args);
   
   //and update time struct
   memcpy(&time_before, &time_after, sizeof(time_before));
}

#endif /*LOG_H_*/
