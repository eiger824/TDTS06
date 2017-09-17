#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "url_filtering.h"

const char* URL_BLACKLIST[] =
{
   /* About SpongeBob */
   "http://spongebob.wikia.com/",
   "https://www.facebook.com/spongebob/",
   "https://twitter.com/spongebob/",
   /* About Britney Spears */
   "https://britneyspears.com/",
   "https://twitter.com/britneyspears/",
   "https://www.facebook.com/britneyspears/",
   "https://www.instagram.com/britneyspears/",
   "https://www.youtube.com/user/BritneySpearsVEVO/",
   /* About Paris Hilton*/
   "https://parishilton.com/",
   "https://www.instagram.com/parishilton/",
   "https://www.facebook.com/ParisHiltonEntertainment/",
   "https://twitter.com/parishilton/",
   "https://www.youtube.com/user/ParisHilton/",
   /* About Norrköping */
   "www.norrkoping.se/",
   "www.nt.se/",
   "ifknorrkoping.se/",
   "norrkopingnews.se/",
   /* Zero-terminate */
   0
};

const char* TOPIC_BLACKLIST[] =
{
   "spongebob",
   "britneyspears",
   "parishilton",
   "norrkoping",
   /* Zero-terminate */
   0
};

int ub_url_in_blacklist(const char *url)
{
   char **c = URL_BLACKLIST;
   while (*c != 0)
   {
      if (!strcmp(url,*c++))
         return 0;
   }
   return -1;
}

int ub_url_matches(const char* url)
{
   //check with strstr if we get some subindex
   char **c = TOPIC_BLACKLIST;
   char *match;
   while (*c != 0)
   {
      if ((match = strstr(url, *c)) != NULL)
      {
         //found at position (match - url)
         return 0;
      }
      *c++;
   }
   return -1;
}

void ub_url_to_lower(char* new_url, const char *url)
{
   unsigned i;
   memcpy(new_url, url, strlen(url)+1);
   for (i=0; i<strlen(url); ++i)
   {
      if (65 <= url[i] && url[i] <= 90) new_url[i] = url[i] + 32;
      else if (url[i] == 'ä') new_url[i] = 'a';
      else if (url[i] == 'ö') new_url[i] = 'o';
      else if (url[i] == 'å') new_url[i] = 'a';
   }
}

int ub_url_extract(char* url, const char* msg)
{
   unsigned i;
   for (i=4; msg[i] != ' '; ++i)
   {
      url[i-4] = msg[i];
   }
   url[i+1-4] = '\0';
   //Last check
   if (url)
      return 0;
   else
      return -1;
}
