/*
 * Print.h
 *
 * Created: 31/10/2014 17:48:22
 *  Author: David
 */ 


#ifndef PRINT_H_
#define PRINT_H_

#include "ecv.h"
#undef array
#undef result
#include "asf.h"
#define array _ecv_array
#define result _ecv_result

#include <cstddef>
#include <cstring>
#include <cmath>

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

class Print
{
private:
	int write_error;
	size_t printNumber(unsigned long, uint8_t);
	size_t printFloat(double, uint8_t);
protected:
	void setWriteError(int err = 1) { write_error = err; }
public:
	Print() : write_error(0) {}
	
	int getWriteError() { return write_error; }
	void clearWriteError() { setWriteError(0); }
	
	virtual size_t write(uint8_t) = 0;
	size_t write(const char *str) {
		if (str == NULL) return 0;
		return write((const uint8_t *)str, strlen(str));
	}
	virtual size_t write(const uint8_t *buffer, size_t size);
	size_t write(const char *buffer, size_t size) {
		return write((const uint8_t *)buffer, size);
	}
	
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
