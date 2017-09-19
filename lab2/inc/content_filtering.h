#ifndef CONTENT_FILTERING_H_
#define CONTENT_FILTERING_H_

/** Function:    cb_page_matches
    Description: Determines if the contents of the HTTP response page
                 match any of the topic blacklist tokens
    @param msg:  Pointer to the HTTP response sent from the remote
                 web-server that is going to be content-analyzed
    Return:      0 if the @msg gets a match, -1 othewise
*/
int cb_page_matches(const char* msg);


/** Function:    cb_page_permitted
    Description: Given an HTTP response buffer, possibly including
                 the contents of the response, this function determines
                 if the response should be forwarded to the client or
                 not. In case of the latter, the cb_send_redirect
                 function shall be used to send an HTTP redirect
                 response to the client
    @param msg:  Pointer to the HTTP response sent from the remote
                 web-server that is going to be content-analyzed
    Returns:     0 if the page is suitable, -1 otherwise
 */
int cb_page_permitted(const char* msg);

#endif /* CONTENT_FILTERING_H_ */

