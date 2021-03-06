#ifndef HTTP_H_
#define HTTP_H_

#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/** Function:      check_if_http_get
    Description:   Given an input buffer, this function determines if it
                   is an HTTP GET header
    @param buffer: The input buffer to analyze
    Returns:       True if @buffer is an HTTP GET header, False otherwise
 */
bool check_if_http_get(const char* buffer)
{
   return buffer[0] == 'G';
}

/** Function:      check_if_http_connect
    Description:   Given an input buffer, this function determines if it
                   is an HTTP CONNECT header
    @param buffer: The input buffer to analyze
    Returns:       True if @buffer is an HTTP CONNECT header, False otherwise
 */
bool check_if_http_connect(const char* buffer)
{
   return buffer[0] == 'C';
}

/** Function:      check_if_http_post
    Description:   Given an input buffer, this function determines if it
                   is an HTTP POST header
    @param buffer: The input buffer to analyze
    Returns:       True if @buffer is an HTTP POST header, False otherwise
 */
bool check_if_http_post(const char* buffer)
{
   return buffer[0] == 'P';
}


/** Function:                  extract_http_info
    Description:               Given an input buffer, this function determines if
                               it is an HTTP header or not, if it is then it deter-
                               mines whether it is a request/response header, thus
                               setting either the hostname to connect to or the
                               length and type of the contents to receive. It also
                               calculates the length of the HTTP header.

    @param msg:                The input buffer to analyze
    @param hostname:           Pointer to the extracted hostname (if HTTP request)
    @param method_type:        Pointer to the updated specified the HTTP message type:
                               1 -> HTTP GET
                               2 -> HTTP CONNECT
                               3 -> HTTP POST
    @param content_length:     Pointer to the parsed content length (if HTTP response)
    @param content_type:       Pointer to the type of content in the HTTP response
                                1 -> content is TEXT type
                                0 -> content is NOT TEXT type
                               -1 -> content type entry is missing or not an HTTP
                                     header
    @param http_header_length: Pointer to the obtained HTTP header's length in bytes
    Returns:                   -2 -> NOT an HTTP header
                               -1 -> Corrupt HTTP header ending
                                0 -> Success (if either @msg is an HTTP request, in
                                     which case @hostname is updated, or if @msg is
                                     an HTTP response, in which @content_length and
                                     @content_type are updated)
 */
int extract_http_info(const char* msg,
                      char* hostname,
                      int* method_type,
                      int *content_length,
                      int *content_type,
                      int *http_header_length)
{
   char *token = "HTTP/";
   char *match;
   char first_line[500];
   unsigned i, j;
   int ret, index;


   // Safety check. HTTPS messages might be super weird
   for (i=0; (msg[i] != '\r' && msg[i] != '\0' && msg[i] != '\n'); ++i)
   {
      first_line[i] = msg[i];
   }
   first_line[i] = '\0';

   // Check if http header
   match = strstr(first_line, token);

   if (match != NULL) //HTTP header
   {
      // Check if the request is complete
      char *header_end = "\r\n\r\n";
      match = strstr(msg, header_end);


      if (match == NULL)
      {
         return -1;
      }
      else
      {
         *http_header_length = match - msg + 4;
      }
      // Look for hostname
      if ((ret = parse_hostname(hostname, msg)) < 0)
      {
         //http response (hostname NOT updated)
         char *length = "Content-Length: ";
         char *type = "Content-Type: ";

         //Content-length
         match = strstr(msg, length);
         if (match != NULL) //found a match
         {
            index = match - msg + strlen(length);
            char len[20];
            for (j=index; msg[j] != '\r'; ++j)
            {
               len[j-index] = msg[j];
            }
            len[j-index] = '\0';
            *content_length = atoi(len);
         }
         else
         {
            *content_length = -1;
            //return -1;
         }

         //Content-type
         match = strstr(msg, type);
         if (match != NULL)
         {
            index = match - msg + strlen(type);
            char typ[50];
            for (j=index; msg[j] != '\r'; ++j)
            {
               typ[j-index] = msg[j];
            }
            typ[j-index] = '\0';
            if (strstr(typ, "text") != NULL)
            {
               *content_type = 1;
            }
            else
            {
               *content_type = 0;
            }
         }
         else
         {
            *content_type = -1;
            //return -1;
         }

         return 0; //success
      }
      else
      {
         //update method-type (hostname is already updated)
         *method_type = ret;
         *content_length = -1;
         *content_type = -1;
         return 0; //request
      }
   }
   else //Not found, not HTTP header
   {
      *content_length = -1;
      *content_type = -1;
      return -2;
   }
}


/** Function:        setup_host_get_connection
    Description:     Given an input hostname, it attempts to establish a connection
                     to the server specified in @servinfo through socket defined by
                     sockfdp
    @param hostname: Pointer to the string holding the hostname
    @param servinfo: Pointer to the addrinfo struct containig the server information
    @param sockfdp:  Socket descriptor to make the connection
    Returns:         -1 upon error, 0 otherwise
 */
int setup_host_get_connection(char *hostname,  struct addrinfo *servinfo, int *sockfdp)
{
 
  struct addrinfo hints, *p;
  int ret;

  //Init addrinfo struct
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; // IPv4
  hints.ai_socktype = SOCK_STREAM; //TCP

  //Get address info of the parsed hostname
  if ((ret = getaddrinfo(hostname, "http", &hints, &servinfo)) != 0)
  {
    return -1;
  }

  // loop through all the results and connect to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next)
  {
    if ((*sockfdp = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1)
    {
      perror("socket");
      return -1;
      //continue;
    }

    if (connect(*sockfdp, p->ai_addr, p->ai_addrlen) == -1)
    {
      perror("connect");
      //close(sockfdp);
      return -1;
      //break;
    }

    break; // if we get here, we must have connected successfully
  }

  if (p == NULL)
  {
    // looped off the end of the list with no connection
    perror("failed to connect");
    return -1;
  }

  return 0;
}


/** Function:        setup_host_connect_connection
    Description:     Given an input hostname, it attempts to establish a connection
                     to the server specified in @servinfo through socket defined by
                     sockfdp
                     @param hostname: Pointer to the string holding the hostname
                     @param servinfo: Pointer to the addrinfo struct containig the server information
                     @param sockfdp:  Socket descriptor to make the connection
                     Returns:         -1 upon error, 0 otherwise
*/
int setup_host_connect_connection(char *hostname,  struct addrinfo *servinfo, int *sockfdp)
{
   struct addrinfo hints, *p;
   int ret;
   //Init addrinfo struct
   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_INET; // IPv4
   hints.ai_socktype = SOCK_STREAM; //TCP

   //Get address info of the parsed hostname
   if ((ret = getaddrinfo(hostname, "https", &hints, &servinfo)) != 0)
   {
      return -1;
   }

   // loop through all the results and connect to the first we can
   for(p = servinfo; p != NULL; p = p->ai_next)
   {
      if ((*sockfdp = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
      {
         perror("socket");
         return -1;
         //continue;
      }

      if (connect(*sockfdp, p->ai_addr, p->ai_addrlen) == -1)
      {
         perror("connect");
         //close(sockfdp);
         return -1;
         //break;
      }

      break; // if we get here, we must have connected successfully
   }

   if (p == NULL)
   {
      // looped off the end of the list with no connection
      perror("failed to connect");
      return -1;
   }

   return 0;
}


#endif /* HTTP_H_ */
