#ifndef _ReverseWords_h_
#define _ReverseWords_h_

#include <string.h>


void ReverseWords(char *a_string)
{
	if (a_string == 0)
	{
		return;
	}

	size_t length = strlen(a_string);

	for (uint32_t i = 0; i < length / 2; ++i)
	{
		std::swap(a_string[i], a_string[length - i - 1]);
		//char temp = a_string[i];
		//a_string[i] = a_string[length - 1 - i];
		//a_string[length - 1 - i] = temp;
	}

    uint32_t wordStart = 0;
	bool hasWord = false;
	for (uint32_t i = 0; i < length; ++i)
	{
		if (hasWord)
		{
			if (a_string[i] == ' ')
			{
                uint32_t wordLength = i - wordStart;
				hasWord = false;
				for (uint32_t w = wordStart; w < wordStart + (wordLength / 2); ++w)
				{
					std::swap(a_string[w], a_string[i - (w - wordStart) - 1]);
				}
			}
		}
		else
		{
			if (a_string[i] != ' ')
			{
				hasWord = true;
				wordStart = i;
			}
		}
	}
	if (hasWord)
	{
		for (uint32_t w = wordStart; w < length; ++w)
		{
			std::swap(a_string[w], a_string[length - (w - wordStart) - 1]);
		}
	}
}



















// Reverses the order of the words in a string but not the letters within the words.
// Assumes words are separated by spaces.
// Does not allocate any extra memory. It is O(n).
void zReverseWords(char *a_string)
{
    if(a_string == NULL)
    {
        return;
    }
    size_t len = strlen(a_string);

    // reverse the entire string
    size_t halflen = len >> 1;
    for(size_t i = 0; i < halflen; ++i)
    {
        std::swap(a_string[i], a_string[len - i - 1]);
    }

    // Go through the string from the start looking for spaces to indicate the end of the current word. 
    // When one is found, reverse the characters in that word, and move on to the next word.
    for(uint32_t i = 0; i < len; ++i)
    {
        uint32_t start = i;
        while(a_string[i] != ' ' && a_string[i] != 0)
        {
            ++i;
        }
        uint32_t halfWordLen = (i - start) >> 1;
        for(uint32_t x = 0; x < halfWordLen; ++x)
        {
            std::swap(a_string[start + x], a_string[i - x - 1]);
        }
    }
}

#endif
