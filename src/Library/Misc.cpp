/*
 * Misc.cpp
 *
 * Created: 14/11/2014 19:58:50
 *  Author: David
 */ 

#include "ecv.h"
#include "Misc.hpp"

void safeStrncpy(char* array dst, const char* array src, size_t n)
{
	while (*src != 0 && n > 1)
	{
		*dst++ = *src++;
		--n;
	}
	*dst = 0;
}

// End
