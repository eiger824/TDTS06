#ifndef UTILS_H_
#define UTILS_H_


/** Function:        text_to_lower
    Description:     Given an input text, it converts all its [A-Z] characters to lowercase
    @param new_text: Pointer to the buffer where the lower-cased string is going to be stored
    @param text:     Pointer to the input text
    @param bytes:    Number of bytes to transform (in order to avoid 0 bytes which may be
                     contained in @text)
    Returns:         The lower-cased text contained in @new_text
 */
void text_to_lower(char *new_text, const char* text, int bytes);

#endif /* UTILS_H_ */
