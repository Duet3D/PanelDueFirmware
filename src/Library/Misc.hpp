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
#undef array
#undef result
#undef value

#undef min
#undef max

#define ARRAY_SIZE(_x) (sizeof(_x)/sizeof(_x[0]))

// Safe version of strncpy that ensures that the destination is always null-terminated on return
void safeStrncpy(char* _ecv_array dst, const char* _ecv_array src, size_t n)
pre(n != 0; _ecv_isNullTerminated(src); dst.upb >= n)
post(_ecv_isNullTerminated(dst));

// Return true if string a is the same as or starts with string b
bool stringStartsWith(const char* _ecv_array a, const char* _ecv_array b)
pre(_ecv_isNullTerminated(a); _ecv_isNullTerminated(b));

// If the text starts with decimal digits followed by underscore, skip that bit
const char * _ecv_array SkipDigitsAndUnderscore(const char * _ecv_array text);

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
