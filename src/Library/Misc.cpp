/*
 * Misc.cpp
 *
 * Created: 14/11/2014 19:58:50
 *  Author: David
 */ 

#include <cctype>
#include "Misc.hpp"

// Safe version of strncpy that ensures that the destination is always null-terminated on return
void safeStrncpy(char* _ecv_array dst, const char* _ecv_array src, size_t n)
{
	while (*src != 0 && n > 1)
	{
		*dst++ = *src++;
		--n;
	}
	*dst = 0;
}

// Return true if string a is the same as or starts with string b
bool stringStartsWith(const char* _ecv_array a, const char* _ecv_array b)
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
const char * _ecv_array SkipDigitsAndUnderscore(const char * _ecv_array text)
{
	const char * const _ecv_array originalText = text;
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
