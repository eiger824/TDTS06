#ifndef URL_FILTERING_H_
#define URL_FILTERING_H_

/* Simple blacklist with non-allowed URLS */
extern const char* URL_BLACKLIST[];

/** Function:    ub_url_in_blacklist
    Description: Given an input url, it checks if it is contained
                 in the blacklist of URLs
    @param url:  Pointer to the url string to check
    Returns:     0 if @url is in the blacklist, -1 otherwise
 */
int ub_url_in_blacklist(const char* url);


/** Function:    ub_url_matches
    Description: Given an input url, it checks if any of the words
                 contained in the topic blacklist is a substring
                 of that url
    @param url:  Pointer to the url string to check
    Returns:     0 if @url contains any of the blacklisted topics,
                 -1 otherwise
 */
int ub_url_matches(const char* url);


/** Function:       ub_url_to_lower
    Description:    Given an input url, it turns every character into
                    lower case (applies only [A-Z]), and it case of
                    finding non-english characters (e.g. ä,ö,å) it
                    converts them to its english-equivalent(a,o,a)
    @param new_url: Pointer to store the new url
    @param url:     Pointer to the url string to check
    Returns:        nothing
 */
void ub_url_to_lower(char* new_url, const char* url);


/** Function:    ub_url_extract
    Description: Given an input buffer, extracted from the HTTP GET
                 header, this function extracts the url to fetch
    @param url:  Pointer to store the parsed url
    @param msg:  Pointer to the message string containing the HTTP
                 request
    Returns:     0 if a valid string is parsed, -1 otherwise
 */
int ub_url_extract(char* url, const char* msg);


/** Function:    ub_url_permitted
    Description: This function encapsulates the above-defined functions,
                 i.e., given an HTTP request it evaluates if the requested
                 page is to be displayed to the client or not
    @param msg:  Pointer to the HTTP request received from the client
    Returns:     0 if the url is not forbidden, -1 otherwise
 */
int ub_url_permitted(const char* msg);

#endif /* URL_FILTERING_H_ */
