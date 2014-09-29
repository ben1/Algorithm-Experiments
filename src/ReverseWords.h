#ifndef _ReverseWords_h_
#define _ReverseWords_h_

#include <string.h>

// Reverses the order of the words in a string but not the letters within the words.
// Assumes words are separated by spaces.
// Does not allocate any extra memory. It is O(n).
void ReverseWords(char *a_string)
{
    if(a_string == NULL)
    {
        return;
    }
    int len = strlen(a_string);

    // reverse the entire string
    int halflen = len >> 1;
    for(int i = 0; i < halflen; ++i)
    {
        std::swap(a_string[i], a_string[len - i - 1]);
    }

    // Go through the string from the start looking for spaces to indicate the end of the current word. 
    // When one is found, reverse the characters in that word, and move on to the next word.
    for(int i = 0; i < len; ++i)
    {
        int start = i;
        while(a_string[i] != ' ' && a_string[i] != 0)
        {
            ++i;
        }
        int halfWordLen = (i - start) >> 1;
        for(int x = 0; x < halfWordLen; ++x)
        {
            std::swap(a_string[start + x], a_string[i - x - 1]);
        }
    }
}

#endif
