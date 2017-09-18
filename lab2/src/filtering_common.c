#include <stdio.h>
#include <unistd.h>
#include <string.h>

/* #include "definitions.h" */
#include "filtering_common.h"

const char* TOPIC_BLACKLIST[] =
{
   "spongebob",
   "britneyspears",
   "parishilton",
   "norrkoping",
   /* Zero-terminate */
   0
};


int proxy_send_redirect(int fd, TYPE type)
{
   char redirect[100000];
   char content_length[50];
   char error_file[500];
   char line[100];

   strcpy(redirect, "HTTP/1.1 302 Found\r\n");
   strcat(redirect, "Content-Type: text/html\r\n");

   FILE *fp;
   if (type == URL_BASED)
      fp = fopen("./data/error_url.html", "r");
   else
      fp = fopen("./data/error_content.html", "r");
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
