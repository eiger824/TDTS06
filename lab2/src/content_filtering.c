#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "filtering_common.h"
#include "content_filtering.h"

int cb_page_matches(const char* msg)
{
   char **c = TOPIC_BLACKLIST;
   char *match;
   while (*c != 0)
   {
      if ((match = strstr(msg, *c)) != NULL)
      {
         return 0;
      }
      *c++;
   }
   return -1;
}

int cb_page_permitted(const char* msg)
{
   //return the opposite of cb_page_matches
   return -(!cb_page_matches(msg));
}

