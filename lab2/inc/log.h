#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define ERROR 0
#define INFO 1

#define log_info(...)      print(INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...)     print(ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_set(X)         (((X) != 0 && (X) != 1) ? (LOG_ENABLED = 0) : (LOG_ENABLED = (X)))
#define log_get()          (LOG_ENABLED)
#define log_init()         if (LOG_ENABLED) {           \
      if (pthread_mutex_init(&log_lock, NULL) != 0) {   \
         perror("Mutex init failed");                   \
      } else {                                          \
         log_info("Log mutex init success!");           \
      }                                                 \
   }


struct timeval time_before, time_after, time_result;
static unsigned LOG_ENABLED = 0;
pthread_mutex_t log_lock;

void print(unsigned level,       /* level: ERROR (stderr) / INFO (stdout) */
           const char* filename, /* filename: The current file where the logging line is */
           unsigned line,        /* line: The current line where the logging line is */
           const char* msg, ...) /* msg: Formated message with following (variadic) args */
{
   if (LOG_ENABLED)
   {
      pthread_mutex_lock(&log_lock);
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
         fprintf(stderr, "[%s%s] (%s:%d) ", time_str, usecs_str, filename, line);
      else
      {
         printf("[%s%s] (%s:%d) ", time_str, usecs_str, filename, line);
      }
   
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
      pthread_mutex_unlock(&log_lock);
   }
}

#endif /*LOG_H_*/
