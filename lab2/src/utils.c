#include <string.h>
#include <ctype.h>

#include "utils.h"

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
