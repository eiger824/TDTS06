#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"
#include "http.h"

#define NORM "\x1B[0m"
#define MARK "\x1B[31m"

typedef unsigned char u8_t;

void text_to_lower(char *text, int bytes)
{
   int i;
   u8_t c;   
   for (i=0; i<bytes; ++i)
   {
      c = (u8_t)text[i];
       if (c == 'å' || c == 'Å') text[i] = 'a';
       else if (c == 'ö' || c == 'Ö') text[i] = 'o';
       else if (c == 'ä' || c == 'Ä') text[i] = 'a';
       else text[i] = tolower(text[i]);
   }
   text[bytes] = '\0';
}

void text_trim_whitespaces(char *text, int n)
{
   int i;
   for (i=0; i<n; ++i)
   {
      if (text[i] == ' ' || text[i] == '\r' || text[i] == '\n')
      {
         memmove(text+i, text+i+1, n-i-1);
         --n;
         text[n] = '\0';
         //decrement i not to skip new current position
         --i;
      }
   }
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
      if (check_if_http_get(buffer)) //GET
         return 1;
      else if (check_if_http_connect(buffer)) //CONNECT
         return 2;
      else //POST
         return 3;
   }
   else
   {
      return -1;
   }
}

void hexify(char* buffer, int n)
{
   int i, j, start = 0, end = 0;
   for (i=0; i<n; ++i)
   {
      if (i > 0 && i%27 == 0)
      {
         end = i;
         printf(" (bytes %d - %d)\n", start, end);
         start = end+1;
      }
      printf("%s%s%x%s "
             ,(buffer[i] == '\n' || buffer[i] == '\r')?MARK:""
             ,((unsigned char)buffer[i]<16)?"0":""
             ,(unsigned char)buffer[i]
             ,(buffer[i] == '\n' || buffer[i] == '\r')?NORM:"");

   }
   if (i % 27 != 0)
   {
      for (j=0; j<3*(27-(i%27)); ++j)
         printf(" ");
      printf(" (bytes %d - %d)\n", start, i);
   }
   else
      printf("\n");
}

void print_data(char* buffer, int n, unsigned hex)
{
   int i;
   printf("============================Buffer");
   printf("(%d bytes)==================\n", n);
   if (hex)
   {
      hexify(buffer, n);
   }
   else
   {
      for (i=0; i<n; ++i)
      {
         if (32 <= buffer[i])
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
            printf("%#x ", (unsigned char)buffer[i]);
         }
      }
      printf("\n");
   }
   printf("=======================================");
   printf("========================================\n");
}


char* get_hostname(char* hostname)
{
   unsigned i, hex=0;

   for (i=0; i<strlen(hostname); ++i)
   {
      if (hostname[i] < 33 || hostname[i] > 126)
      {
         //Some weird character found - print it hexadecimal
         hex = 1;
         break;
      }
   }
   char *hn;
   if (hex)
   {
      hn = (char*) malloc(3*strlen(hostname));
      strcpy(hn, "");
      //Generate hex representation of hostname
      for (i=0; i<strlen(hostname); ++i)
      {
         char tmp[4];
         sprintf(tmp, "%x-", (unsigned char)hostname[i]);
         tmp[3] = '\0';
         strcat(hn, tmp);
      }
      hn[3*i-1] = '\0';
   }
   else
   {
      hn = (char*) malloc(strlen(hostname)+1);
      memcpy(hn, hostname, strlen(hostname)+1);
      hn[strlen(hostname)] = '\0';
   }
   return hn;
}
