#ifndef FILTERING_COMMON_H_
#define FILTERING_COMMON_H_

//#include "definitions.h"

/* Simple topic blacklist, with no capital letters
   and in english alphabet */
extern const char* TOPIC_BLACKLIST[];

typedef enum {
   URL_BASED,
   CONTENT_BASED
} TYPE;

/** Function:    proxy_send_redirect
    Description: This function sends an HTTP redirect back to the client
                 with an HTML file stating that the client is not allowed
                 to display the requested contents
    @param fd:   The socket file descriptor used by the client to make the
                 HTTP request
    @param type: The type of redirect to send i.e. if it is URL-based or
                 content-based
    Returns:     The number of written bytes to the client if the HTTP
                 redirect operation was sucessful, -1 otherwise
 */
int proxy_send_redirect(int fd, TYPE type);

#endif /* FILTERING_COMMON_H_ */
