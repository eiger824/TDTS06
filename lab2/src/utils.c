#include <string.h>
#include <ctype.h>

#include "utils.h"

void text_to_lower(char *new_text, const char* text, int bytes)
{
   unsigned i;
   memcpy(new_text, text, bytes);
   for (i=0; i<bytes; ++i)
   {
       if (text[i] == '�' || text[i] == '�') new_text[i] = 'a';
       else if (text[i] == '�' || text[i] == '�') new_text[i] = 'o';
       else if (text[i] == '�' || text[i] == '�') new_text[i] = 'a';
       else new_text[i] = tolower(text[i]);
   }
   new_text[bytes] = '\0';
}

