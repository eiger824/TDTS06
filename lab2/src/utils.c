#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"
#include "http.h"

void text_to_lower(char *text, int bytes)
{
   unsigned i;
   for (i=0; i<bytes; ++i)
   {
       if (text[i] == '�' || text[i] == '�') text[i] = 'a';
       else if (text[i] == '�' || text[i] == '�') text[i] = 'o';
       else if (text[i] == '�' || text[i] == '�') text[i] = 'a';
       else text[i] = tolower(text[i]);
   }
   text[bytes] = '\0';
}

void text_trim_whitespaces(char *text, int n)
{
   unsigned i;
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
            printf("%#x", buffer[i]);
         }
      }
   }
   printf("==============================================\n");
}
