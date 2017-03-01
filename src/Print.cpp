/*
 * Print.cpp
 *
 * Created: 31/10/2014 17:52:30
 *  Author: David
 * Adapted from Arduino Print module
 */ 

#include "Print.hpp"

#include <cstdint>
#include <climits>
#include <cmath>

// Print a string stopping at null or newline, returning the number of bytes consumed
size_t Print::print(const char str[])
{
	size_t w = 0;
	if (str != nullptr)
	{
		while (*str != 0 && *str != '\n')
		{
			w += write(*str);
			++str;
		}
	}
	return w;
}

size_t Print::print(long n, unsigned int base)
{
	if (base == 0)
	{
		return write((uint8_t)n);
	}
	else if (base == 10)
	{
		if (n < 0)
		{
			const size_t t = write('-');
			return printNumber(-n, 10) + t;
		}
		return printNumber(n, 10);
	}
	else
	{
		return printNumber((uint32_t)n, base);
	}
}

// Private Methods /////////////////////////////////////////////////////////////

size_t Print::printNumber(uint32_t n, unsigned int base)
{
	char buf[CHAR_BIT * sizeof(long) + 1];			// the largest buffer is needed when base=2
	char *str = &buf[sizeof(buf) - 1];

	*str = '\0';

	// prevent crash if called with base == 1
	if (base < 2)
	{
		base = 10;
	}

	do
	{
		const uint32_t m = n;
		n /= base;
		const char c = m - (base * n);
		*--str = (c < 10) ? c + '0' : c + ('A' - 10);
	} while(n != 0);

	return print(str);
}

size_t Print::printFloat(float number, unsigned int digits)
{
	if (std::isnan(number)) return print("nan");
	if (std::isinf(number)) return print("inf");
	
	// Handle negative numbers
	const bool neg = (number < 0.0);
	if (neg)
	{
		number = -number;
	}

	// Round correctly so that print(1.999, 2) prints as "2.00"
	unsigned int roundVal = 2;
	for (uint8_t i = 0; i < digits; ++i)
	{
		roundVal *= 10;
	}
	
	number += 1.0/(float)roundVal;
	if (number > (float)UINT32_MAX)
	{
		return print ("ovf");
	}

	size_t n = 0;
	if (neg)
	{
		n += write('-');
	}
	
	// Extract the integer part of the number and print it
	const uint32_t int_part = (uint32_t)number;
	float remainder = number - (float)int_part;
	n += printNumber(int_part, DEC);

	// Print the decimal point, but only if there are digits beyond
	if (digits != 0)
	{
		n += print("\xC2\xB7");		// Unicode middle dot

		// Extract digits from the remainder one at a time
		do
		{
			remainder *= 10.0;
			const int toPrint = int(remainder);
			n += print(toPrint);
			remainder -= toPrint;
			--digits;
		} while (digits != 0);
	}
	
	return n;
}

// End
