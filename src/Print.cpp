/*
 * Print.cpp
 *
 * Created: 31/10/2014 17:52:30
 *  Author: David
 * Adapted from Arduino Print module
 */ 

#include "Print.hpp"
#include <climits>

/* default implementation: may be overridden */
size_t Print::write(const uint8_t *buffer, size_t size)
{
	size_t n = 0;
	while (size != 0)
	{
		n += write(*buffer++);
		--size;
	}
	return n;
}

size_t Print::print(const char str[])
{
	return write(str);
}

size_t Print::print(char c)
{
	return write(c);
}

size_t Print::print(unsigned char b, int base)
{
	return print((unsigned long) b, base);
}

size_t Print::print(int b, int base)
{
	return print((long) b, base);
}

size_t Print::print(unsigned int b, int base)
{
	return print((unsigned long) b, base);
}

size_t Print::print(long n, int base)
{
	if (base == 0)
	{
		return write(n);
	}
	else if (base == 10)
	{
		if (n < 0)
		{
			const size_t t = print('-');
			n = -n;
			return printNumber(n, 10) + t;
		}
		return printNumber(n, 10);
	}
	else
	{
		return printNumber(n, base);
	}
}

size_t Print::print(unsigned long n, int base)
{
	if (base == 0)
	{
		return write(n);
	}
	return printNumber(n, base);
}

size_t Print::print(double n, int digits)
{
	return printFloat(n, digits);
}

// Private Methods /////////////////////////////////////////////////////////////

size_t Print::printNumber(uint32_t n, uint8_t base)
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

	return write(str);
}

size_t Print::printFloat(double number, uint8_t digits)
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
	
	number += 1.0/(double)roundVal;
	if (number > (double)UINT32_MAX)
	{
		return print ("ovf");
	}

	size_t n = 0;
	if (neg)
	{
		n += print('-');
	}
	
	// Extract the integer part of the number and print it
	const uint32_t int_part = (uint32_t)number;
	double remainder = number - (double)int_part;
	n += print(int_part);

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
