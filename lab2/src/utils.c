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
       if (text[i] == 'å' || text[i] == 'Å') text[i] = 'a';
       else if (text[i] == 'ö' || text[i] == 'Ö') text[i] = 'o';
       else if (text[i] == 'ä' || text[i] == 'Ä') text[i] = 'a';
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
   unsigned start = 0;
   for (i=0; i<n; ++i)
   {
      //check for end line
      if (i > 0 && i % 27 == 0)
      {
         printf("(bytes %d - %d)\n", start, i);
         start = i + 1;
      }
      //caution with checking i+1 on last element
      if (i < n-1)
      {
         if (buffer[i] == 0xd && buffer[i+1] == 0xa)
         {
            printf(" (CR + NL)\n");
            ++i;
         }
      }
      if (buffer[i] < 16)
      {
         printf("0%x ", buffer[i]);
      }
      else
      {
         printf("%x ", buffer[i]);
      }
   }
   //write the last indication
   for (unsigned j=0; j<3*(27-(i%27)); ++j)
      printf(" ");
   printf("(bytes %d - %d)", start, i);
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
            printf("%#x", buffer[i]);
         }
      }
   }
   printf("\n=======================================");
   printf("========================================\n");
}
