/*
 * Misc.h
 *
 * Created: 14/11/2014 19:56:03
 *  Author: David
 */ 


#ifndef MISC_H_
#define MISC_H_

#include <cstddef>

#define ARRAY_SIZE(_x) (sizeof(_x)/sizeof(_x[0]))

void safeStrncpy(char* array dst, const char* array src, size_t n)
pre(n != 0; _ecv_isNullTerminated(src); dst.upb >= n);

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
