#include <stdio.h>
#include <unistd.h>
#include <string.h>
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
   /* About Norrk�ping */
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
      else if (url[i] == '�') new_url[i] = 'a';
      else if (url[i] == '�') new_url[i] = 'o';
      else if (url[i] == '�') new_url[i] = 'a';
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

int ub_url_permitted(const char* msg)
{
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

int ub_send_redirect(int fd)
{
   char redirect[100000];
   char content_length[50];
   char error_file[500];
   char line[100];

   strcpy(redirect, "HTTP/1.1 302 Found\r\n");
   strcat(redirect, "Content-Type: text/html\r\n");

   FILE *fp = fopen("./data/error_url.html", "r");
   if (fp != NULL)
   {
      while (!feof(fp))
      {
         if (fgets(line, 100, fp) != NULL)
         {
            strcat(error_file, line);
         }
      }
      fclose(fp);
   }
   else
   {
      perror("Error opening file");
      return -1;
   }
   content_length[sprintf(content_length, "Content-Length: %d\r\n", strlen(error_file))] = '\0';
   strcat(redirect, content_length);
   strcat(redirect, "\r\n\n");
   strcat(redirect, error_file);
   
   int n;
   if (fd)
   {
      n = write(fd, redirect, strlen(redirect));
      if (n<0)
      {
         perror("write");
         return -1;
      }
      else
      {
         close(fd);
         return n;
      }
   }
   return -1;
}