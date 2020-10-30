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

namespace SerialIo
{
	void Init(uint32_t baudRate);
	void SendChar(char c);
	size_t Sendf(const char *fmt, ...) noexcept;
	void SendString(const char * _ecv_array s);
	void SendQuoted(const char * _ecv_array s);
	void SendFilename(const char * _ecv_array dir, const char * _ecv_array name);
	void SendInt(int i);
	void CheckInput();
}

#endif /* SERIALIO_H_ */
