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

// If the text starts with decimal digits followed by underscore, skip that bit
const char * _ecv_array SkipDigitsAndUnderscore(const char * _ecv_array text);

#endif /* MISC_H_ */
