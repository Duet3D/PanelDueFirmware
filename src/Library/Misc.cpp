/*
 * Misc.cpp
 *
 * Created: 14/11/2014 19:58:50
 *  Author: David
 */ 

#include "ecv.h"
#include "Misc.hpp"
#include <cctype>

// Safe version of strncpy that ensures that the destination is always null-terminated on return
void safeStrncpy(char* array dst, const char* array src, size_t n)
{
	while (*src != 0 && n > 1)
	{
		*dst++ = *src++;
		--n;
	}
	*dst = 0;
}

// Return true if string a is the same as or starts with string b
bool stringStartsWith(const char* array a, const char* array b)
{
	while (*b != 0)
	{
		if (*a++ != *b++)
		{
			return false;
		}
	}
	return true;
}

// If the text starts with decimal digits followed by underscore, skip that bit
const char * array SkipDigitsAndUnderscore(const char * array text)
{
	const char * const array originalText = text;
	if (isdigit(*text))
	{
		do
		{
			++text;
		} while (isdigit(*text));
		return (*text == '_') ? text + 1 : originalText;
	}
	return originalText;
}

// End
