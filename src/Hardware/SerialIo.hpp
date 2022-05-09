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
#include <General/String.h>
#include "ecv.h"
#undef array
#undef result
#undef value

namespace SerialIo
{
	struct SerialIoCbs
	{
		void (*StartReceivedMessage)(void);
		void (*EndReceivedMessage)(void);
		void (*ProcessReceivedValue)(StringRef id, const char val[], const size_t indices[]);
		void (*ProcessArrayEnd)(const char id[], const size_t indices[]);
		void (*ParserErrorEncountered)(int currentState, const char* id, int errors);
	};

	void Init(uint32_t baudRate, struct SerialIoCbs *callbacks);
	void SetBaudRate(uint32_t baudRate);
	void SendChar(char c);
	void SetCRC16(bool enable);
	size_t Sendf(const char *fmt, ...) __attribute__((format (printf, 1, 0)));
	size_t Dbg(const char *fmt, ...) __attribute__((format (printf, 1, 0)));
	void SendFilename(const char * _ecv_array dir, const char * _ecv_array name);
	void SendFloat(float f);
	void CheckInput();
}

#endif /* SERIALIO_H_ */
