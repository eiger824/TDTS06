#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#define ERROR 0
#define INFO 1

void print(const char* msg, unsigned level)
{
   time_t current_time;
   char* c_time;
   
   current_time = time(NULL);

   c_time = ctime(&current_time);
   c_time[strlen(c_time)-1] = 0;

   int millisec;
   struct tm* tm_info;
   struct timeval tv;

   gettimeofday(&tv, NULL);

   millisec = lrint(tv.tv_usec/1000.0); // Round to nearest millisec
   if (millisec>=1000) { // Allow for rounding up to nearest second
      millisec -=1000;
      tv.tv_sec++;
   }
   

   char buf[strlen(msg) + 2];
   strcpy(buf, msg);
   strcat(buf, "\n");
   buf[strlen(buf)] = 0;
   if (level)
   {
      printf("[%s.%03d] %s", c_time,millisec, buf);
   }
   else
   {
      fprintf(stderr, "[%s] %s", c_time, msg);
   }
}

#endif /*LOG_H_*/
