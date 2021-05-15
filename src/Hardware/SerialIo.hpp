/*
 * SerialIo.hpp
 *
 * Created: 09/11/2014 09:20:46
 *  Author: David
 */


#ifndef SERIALIO_H_
#define SERIALIO_H_

#include <cstddef>
#include <cstdint>
#include "ecv.h"
#undef array
#undef result
#undef value

namespace SerialIo
{
	void Init(uint32_t baudRate);
	void SendChar(char c);
	size_t Sendf(const char *fmt, ...) __attribute__((format (printf, 1, 0)));
	void SendFilename(const char * _ecv_array dir, const char * _ecv_array name);
	void SendFloat(float f);
	void CheckInput();
}

#endif /* SERIALIO_H_ */
