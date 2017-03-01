/*
 * Print.h
 *
 * Created: 31/10/2014 17:48:22
 *  Author: David
 */ 


#ifndef PRINT_H_
#define PRINT_H_

#include "ecv.h"
#include <cstdint>
#include <cstddef>
#include <cstring>

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

class Print
{
private:
	size_t printNumber(uint32_t, unsigned int);
	size_t printFloat(float, unsigned int);

public:
	Print() {}
	
	virtual size_t write(uint8_t c) = 0;

	size_t print(const char[]);
	size_t print(int n, unsigned int base = DEC) { return print((long)n, base); }
	size_t print(unsigned int n, unsigned int base = DEC) { return printNumber(n, base); }
	size_t print(long n, unsigned int base = DEC);
	size_t print(unsigned long n, unsigned int base = DEC) { return printNumber(n, base); }
	size_t print(float n, unsigned int decimalDigits = 2) { return printFloat(n, decimalDigits); }
	size_t print(double n, unsigned int decimalDigits = 2) { return printFloat((float)n, decimalDigits); }
};

#endif /* PRINT_H_ */
