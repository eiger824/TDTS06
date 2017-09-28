#ifndef UTILS_H_
#define UTILS_H_


/** Function:        text_to_lower
    Description:     Given an input text, it converts all its [A-Z] characters
                     to lowercase
    @param text:     Pointer to the buffer to analyze
    @param bytes:    Number of bytes to transform (in order to avoid 0 bytes
                     which may be
                     contained in @text)
    Returns:         The lower-cased text
 */
void text_to_lower(char *text, int bytes);


/** Function:        text_trim_whitespaces
    Description:     Given an input text, it removes all its whitespace
                     characters
    @param text:     Pointer to the buffer to analyze
    @param n:    Number of bytes to read (in order to avoid 0 bytes
                     contained in @text)
    Returns:         The whitespace-trimmed text
 */
void text_trim_whitespaces(char *text, int n);


/** Function:        parse_hostname
    Description:     Given an input buffer, it extracts the hostname contained
                     in the field Host: of the HTTP request
    @param hostname: Pointer to the string containing the buffer
    @param buffer:   The input buffer to analyze
    Returns:         -2 -> Neither an HTTP GET nor HTTP CONNECT, and @hostname
                           remains intact
                     -1 -> If no "Host:" entry was found in the request, and
                           @hostname remains intact, or if it was actually found
                           but an empty string was obtained
                      1 -> If @buffer is an HTTP GET header, and @hostname has
                           been found
                      2 -> If @buffer is an HTTP CONNECT header, and @hostname has
                           been found
                      3 -> If @buffer is an HTTP POST header, and @hostname has
                           been found
 */
int parse_hostname(char *hostname, const char* buffer);


/** Function:      hexify
    Description:   Given an input buffer and its length in bytes, it prints out
                   its characters in hexadecimal format to the standard output
    @param buffer: The input buffer to print
    @param n:      The number of bytes @buffer has
    Returns:       Nothing
 */
void hexify(char *buffer, int n);


/** Function:      print_data
    Description:   Given an input buffer and its length in bytes, it prints it to
                   the standard output
    @param buffer: The input buffer to print
    @param n:      The number of bytes @buffer has
    @param hex:    If the data should be printed out in hex
    Returns:       Nothing
 */
void print_data(char* buffer, int n, unsigned hex);


/** Function:        get_hostname
    Description:     Given an input hostname, it determines whether its characters
                     are printable and prints it in plain text if they are, or in
                     its hexadecimal representation if not
    @param hostname: A pointer to the hostname to analyze
    Returns:         A pointer to the string containing the result of the operation
*/
char* get_hostname(char* hostname);

#endif /* UTILS_H_ */
