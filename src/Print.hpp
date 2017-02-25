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
	size_t printNumber(unsigned long, uint8_t);
	size_t printFloat(double, uint8_t);

public:
	Print() {}
	
	virtual size_t write(uint8_t c) = 0;

	size_t print(const char[]);
	size_t print(char);
	size_t print(unsigned char, int = DEC);
	size_t print(int, int = DEC);
	size_t print(unsigned int, int = DEC);
	size_t print(long, int = DEC);
	size_t print(unsigned long, int = DEC);
	size_t print(double, int = 2);
};

#endif /* PRINT_H_ */
