/*
 * SerialIo.hpp
 *
 * Created: 09/11/2014 09:20:46
 *  Author: David
 */ 


#ifndef SERIALIO_H_
#define SERIALIO_H_

namespace SerialIo
{
	void Init(uint32_t baudRate);
	void SendChar(char c);
	void SendString(const char * array s);
	void SendFilename(const char * array dir, const char * array name);
	void SendInt(int i);
	void CheckInput();
}

#endif /* SERIALIO_H_ */