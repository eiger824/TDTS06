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

#endif /* UTILS_H_ */
