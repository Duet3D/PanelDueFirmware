/*
 * Misc.cpp
 *
 * Created: 14/11/2014 19:58:50
 *  Author: David
 */ 

#include <cctype>
#include "Misc.hpp"

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
