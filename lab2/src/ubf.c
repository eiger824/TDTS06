#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "filtering_common.h"
#include "ubf.h"
#include "utils.h"

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
   /* About Norrk�ping */
   "www.norrkoping.se/",
   "www.nt.se/",
   "ifknorrkoping.se/",
   "norrkopingnews.se/",
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
   //Safe to provide strlen(url), since it is zero-terminated in
   //@ub_url_extract
   text_to_lower(new_url, strlen(url));
}

int ub_url_extract(char* url, const char* msg)
{
   unsigned i, init;
   char hostname[200];
   parse_hostname(hostname, msg);
   if (msg[0] == 'G') //GET
   {
      init = 4;
   }
   else if (msg[0] == 'P') //POST
   {
      init = 5;
   }
   else if (msg[0] == 'C') // Connect
   {
      init = 8;
   }

   //check if the url is absolute or relative
   if (msg[init] == '/')
   {
      //start by copying the hostname first
      memcpy(url, hostname, strlen(hostname));
      //then store the relative path in subpath
      char subpath[200];
      for (i = init; msg[i] != ' '; ++i)
      {
         subpath[i-init] = msg[i];
      }
      subpath[i-init] = '\0';
      strcat(url, subpath);
   }
   else
   {
      for (i=init; msg[i] != ' '; ++i)
      {
         url[i-init] = msg[i];
      }
      url[i-init] = '\0';
   }

   //Last check
   if (url)
      return 0;
   else
      return -1;
}

int ub_url_permitted(const char* msg)
{
   if (!strlen(msg))
   {
      return -2;
   }
   char url[200];
   char url_lowercase[200];
   //1.) Extract URL from HTTP request
   if (!ub_url_extract(url, msg))
   {
      //2.) Convert it to lowercase
      ub_url_to_lower(url_lowercase, url);
      //3.) Check if that URL is in the URL blacklist
      if (!ub_url_in_blacklist(url_lowercase))
      {
         return 0;
      }
      else
      {
         //3.1) If not, check if it partially matches any
         //topic keywords
         if (!ub_url_matches(url_lowercase))
         {
            return -1;
         }
         else
         {     
            return 0;
         }
      }
   }
   else
   {
      return -1;
   }
}

