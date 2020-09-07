/*
 * Misc.h
 *
 * Created: 14/11/2014 19:56:03
 *  Author: David
 */ 


#ifndef MISC_H_
#define MISC_H_

#include <cstddef>
#include "ecv.h"

#undef min
#undef max

#define ARRAY_SIZE(_x) (sizeof(_x)/sizeof(_x[0]))

// Safe version of strncpy that ensures that the destination is always null-terminated on return
void safeStrncpy(char* array dst, const char* array src, size_t n)
pre(n != 0; _ecv_isNullTerminated(src); dst.upb >= n)
post(_ecv_isNullTerminated(dst));

// Return true if string a is the same as or starts with string b
bool stringStartsWith(const char* array a, const char* array b)
pre(_ecv_isNullTerminated(a); _ecv_isNullTerminated(b));

// If the text starts with decimal digits followed by underscore, skip that bit
const char * array SkipDigitsAndUnderscore(const char * array text);

template<class T> T min(const T& a, const T& b)
{
	return (a < b) ? a : b;
}

template<class T> T max(const T& a, const T& b)
{
	return (a > b) ? a : b;
}

template<class T> T constrain(const T& v, const T& minVal, const T& maxVal)
{
	return (v < minVal) ? minVal : (v > maxVal) ? maxVal : v;
}

#endif /* MISC_H_ */
