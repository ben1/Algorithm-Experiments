#ifndef _GetMostCommonLetter_h_
#define _GetMostCommonLetter_h_

#include <intrin.h>


// Returns the most frequent letter in the string, ignoring case. Does not consider non-letter characters.
// If more than one character has the largest count, the lower value letter is returned.
// Returns 0 if the string is null or empty cannot be processed.
char GetMostCommonLetter(char *a_buf)
{
    if(a_buf == NULL)
    {
        return 0;
    }
    
    // The string is < 1billion characters long, so we can store counts in int32.
    // We only count letters, but including an if statement to check for this will slow down the loop.
    // It's faster to just keep a count of all character values.
    const int numChars = 256;
    unsigned int counts[numChars];

    // set the initial counts to 0
    memset(counts, 0, numChars * sizeof(int));

    // count all characters in the string
    while(*a_buf)
    {
        // no branches, everything should be cached
        counts[*a_buf]++;
        a_buf++;
    }

    // find the letter with the highest count (only 26 iterations)
    int capitalOffset = 'A' - 'a';
    unsigned int bestCount = 0;
    int bestIndex = 0;
    // we only consider actual English letters, no whitespace or punctuation etc.
    for(int i = 'a'; i <= 'z'; ++i)
    {
        // include lowercase and uppercase letters in the total letter count.
        unsigned int totalCount = counts[i] + counts[i + capitalOffset];
        if(totalCount > bestCount)
        {
            bestCount = totalCount;
            bestIndex = i;
        }
    }
    return bestIndex;
}

#endif
