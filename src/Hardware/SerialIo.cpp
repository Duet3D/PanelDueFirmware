/*
 * SerialIo.cpp
 *
 * Created: 09/11/2014 09:20:26
 *  Author: David
 */

#include "SerialIo.hpp"
#include "asf.h"
#include "General/String.h"
#include "General/SafeVsnprintf.h"
#include "PanelDue.hpp"
#define DEBUG (0)
#if DEBUG
# include "MessageLog.hpp"
#endif

const size_t MaxArrayNesting = 4;

#if SAM4S
# define UARTn	UART0
#else
# define UARTn	UART1
#endif


namespace SerialIo
{
	static unsigned int lineNumber = 0;

	// Translation tables for combining characters.
	// The first character in each pair is the character that the combining mark is applied to.
	// The second character is what is translates to if the value is >=0x80, else the value it translates to minus 0x0100.
	const char* _ecv_array const trGrave =		"A\xC0"			"E\xC8" 				"I\xCC"         "O\xD2" "U\xD9"
											"a\xE0"			"e\xE8" 				"i\xEC"			"o\xF2" "u\xF9"			;
	const char* _ecv_array const trAcute =		"A\xC1" "C\x06" "E\xC9" 				"I\xCD" "L\x39" "N\x43" "O\xD3" "R\x54" "S\x5A" "U\xDA" "Y\xDD"	"Z\x79"
											"a\xE1" "c\x07" "e\xE9" 				"i\xED" "l\x39"	"n\x44"	"o\xF3" "r\x55" "s\x5B" "u\xFA" "y\xFD" "z\x7a"	;
	const char* _ecv_array const trCircumflex =	"A\xC2" "C\x08" "E\xCA" "G\x1C" "H\x24" "I\xCE" "J\x34"	"O\xD4" "S\x5C" "U\xDB" "W\x74" "Y\x76"
											"a\xE2" "c\x09" "e\xEA" "g\x1D" "h\x25" "i\xEE" "j\x35"	"o\xF4" "s\x5D" "u\xFB"	"w\x75"	"y\x77"	;
	const char* _ecv_array const trTilde =		"A\xC3"                 				"I\x28" "N\xD1" "O\xD5" "U\x68"
											"a\xE3"                 				"i\x29"	"n\xF1"	"o\xF5"	"u\x69"			;
	const char* _ecv_array const trBreve =		"A\x02"			"E\x14" "G\x1E" 		"I\x2C"			"O\x4E" "U\x6c"
											"a\x03"			"e\x15" "g\x1F" 		"i\x2D"			"o\x4F" "u\x6d"			;
	const char* _ecv_array const trUmlaut =		"A\xC4"			"E\xCB" 				"I\xCF"         "O\xD6" "U\xDC" "Y\x78"
											"a\xE4"			"e\xEB" 				"i\xEF"			"o\xF6" "u\xFC" "y\xFF"	;
	const char* _ecv_array const trCircle =		"A\xC5"															"U\x6E"
											"a\xE5"                                 						"u\x6F"			;
	const char* _ecv_array const trCaron =		"C\x0C"	"D\x0C" "E\x1A" "N\x47" "R\x58" "S\x60" "T\x64" "Z\x7D"
											"c\x0D" "d\x0F" "e\x1B" "n\x48" "r\x59" "s\x61" "t\x65" "z\x7E"					;
	const char* _ecv_array const trCedilla =		"C\xC7"
											"c\xE7"																			;

	// Initialize the serial I/O subsystem, or re-initialize it with a new baud rate
	void Init(uint32_t baudRate)
	{
		uart_disable_interrupt(UARTn, 0xFFFFFFFF);
#if SAM4S
		pio_configure(PIOA, PIO_PERIPH_A, PIO_PA9 | PIO_PA10, 0);	// enable UART 0 pins
#else
		pio_configure(PIOB, PIO_PERIPH_A, PIO_PB2 | PIO_PB3, 0);	// enable UART 1 pins
#endif
		sam_uart_opt uartOptions;
		uartOptions.ul_mck = sysclk_get_main_hz()/2;	// master clock is PLL clock divided by 2
		uartOptions.ul_baudrate = baudRate;
		uartOptions.ul_mode = US_MR_PAR_NO;				// mode = normal, no parity
		uart_init(UARTn, &uartOptions);
#if SAM4S
		irq_register_handler(UART0_IRQn, 5);
#else
		irq_register_handler(UART1_IRQn, 5);
#endif
		uart_enable_interrupt(UARTn, UART_IER_RXRDY | UART_IER_OVRE | UART_IER_FRAME);
	}

	uint16_t numChars = 0;
	uint8_t checksum = 0;

	// Send a character to the 3D printer.
	// A typical command string is only about 12 characters long, which at 115200 baud takes just over 1ms to send.
	// So there is no particular reason to use interrupts, and by so doing so we avoid having to handle buffer full situations.
	void RawSendChar(char c)
	{
		while(uart_write(UARTn, c) != 0) { }
	}

	void SendCharAndChecksum(char c)
	{
		checksum ^= c;
		RawSendChar(c);
		++numChars;
	}

	void SendChar(char c)
	decrease(numChars == 0)
	{
		if (c == '\n')
		{
			if (numChars != 0)
			{
				// Send the checksum
				RawSendChar('*');
				char digit0 = checksum % 10 + '0';
				checksum /= 10;
				char digit1 = checksum % 10 + '0';
				checksum /= 10;
				if (checksum != 0)
				{
					RawSendChar(checksum + '0');
				}
				RawSendChar(digit1);
				RawSendChar(digit0);
			}
			RawSendChar(c);
			numChars = 0;
		}
		else
		{
			if (numChars == 0)
			{
				checksum = 0;
				// Send a dummy line number
				SendCharAndChecksum('N');
				Sendf("%d", lineNumber++);			// numChars is no longer zero, so only recurses once
				SendCharAndChecksum(' ');
			}
			SendCharAndChecksum(c);
		}
	}

	size_t Sendf(const char *fmt, ...)
	{
		va_list vargs;
		va_start(vargs, fmt);
		return vuprintf([](char c) noexcept -> bool {
			if (c != 0)
			{
				SendChar(c);
			}
			return true;
		}, fmt, vargs);
	}

	void SendFilename(const char * _ecv_array dir, const char * _ecv_array name)
	{
		const char* quote = (GetFirmwareFeatures() & quoteFilenames) ? "\"" : "";
		Sendf("%s%s%s%s%s",
				quote,
				dir,
				((dir[strlen(dir)-1] != '/') ? "/" : ""),
				name,
				quote);
	}

	// Receive data processing
	const size_t rxBufsize = 8192;
	static volatile char rxBuffer[rxBufsize];
	static volatile size_t nextIn = 0;
	static size_t nextOut = 0;
	static bool inError = false;

	// Enumeration to represent the json parsing state.
	// We don't allow nested objects or nested arrays, so we don't need a state stack.
	// An additional variable elementCount is 0 if we are not in an array, else the number of elements we have found (including the current one)
	enum JsonState
	{
		jsBegin,			// initial state, expecting '{'
		jsExpectId,			// just had '{' so expecting a quoted ID
		jsId,				// expecting an identifier, or in the middle of one
		jsHadId,			// had a quoted identifier, expecting ':'
		jsVal,				// had ':', expecting value
		jsStringVal,		// had '"' and expecting or in a string value
		jsStringEscape,		// just had backslash in a string
		jsIntVal,			// receiving an integer value
		jsNegIntVal,		// had '-' so expecting a integer value
		jsFracVal,			// receiving a fractional value
		jsEndVal,			// had the end of a string or _ecv_array value, expecting comma or ] or }
		jsCharsVal,			// receiving an alphanumeric value such as true, false, null
		jsError				// something went wrong
	};

	JsonState state = jsBegin;

	// fieldId is the name of the field being received. A '^' character indicates the position of an _ecv_array index, and a ':' character indicates a field separator.
	String<100> fieldId;
	String<300> fieldVal;	// long enough for about 6 lines of message
	size_t arrayIndices[MaxArrayNesting];
	size_t arrayDepth = 0;

	static void RemoveLastId()
	{
#if DEBUG
		MessageLog::AppendMessage(150, "RemoveLastId: %s, len: %d", fieldId.c_str(), fieldId.strlen());
#endif
		size_t index = fieldId.strlen();
		while (index != 0 && fieldId[index - 1] != '^' && fieldId[index - 1] != ':')
		{
			--index;
		}
		fieldId.Truncate(index);
#if DEBUG
		MessageLog::AppendMessage(150, "RemoveLastId: %s, len: %d", fieldId.c_str(), fieldId.strlen());
#endif
	}

	static void RemoveLastIdChar()
	{
#if DEBUG
		MessageLog::AppendMessage("RemoveLastIdChar");
#endif
		if (fieldId.strlen() != 0)
		{
			fieldId.Truncate(fieldId.strlen() - 1);
		}
	}

	static bool InArray()
	{
#if DEBUG
		MessageLog::AppendMessage("InArray");
#endif
		return fieldId.strlen() > 0 && fieldId[fieldId.strlen() - 1] == '^';
	}

	static void ProcessField()
	{
#if DEBUG
		MessageLog::AppendMessage("ProcessField");
#endif
		if (state == jsCharsVal)
		{
			if (fieldVal.Equals("null"))
			{
				fieldVal.Clear();				// so that we can distinguish null from an empty string
			}
		}
		ProcessReceivedValue(fieldId.GetRef(), fieldVal.c_str(), arrayIndices);
		fieldVal.Clear();
	}

	static void EndArray()
	{
#if DEBUG
		MessageLog::AppendMessage("EndArray");
#endif
		ProcessArrayEnd(fieldId.c_str(), arrayIndices);
		if (arrayDepth != 0)			// should always be true
		{
			--arrayDepth;
			RemoveLastIdChar();
		}
	}

	// Look for combining characters in the string value and convert them if possible
	static void ConvertUnicode()
	{
		unsigned int numContinuationBytesLeft = 0;
		uint32_t charVal;
		for (size_t i = 0; i < fieldVal.strlen(); )
		{
			const unsigned char c = fieldVal[i++];
			if (numContinuationBytesLeft == 0)
			{
				if (c >= 0x80)
				{
					if ((c & 0xE0) == 0xC0)
					{
						charVal = (uint32_t)(c & 0x1F);
						numContinuationBytesLeft = 1;
					}
					else if ((c & 0xF0) == 0xE0)
					{
						charVal = (uint32_t)(c & 0x0F);
						numContinuationBytesLeft = 2;
					}
					else if ((c & 0xF8) == 0xF0)
					{
						charVal = (uint32_t)(c & 0x07);
						numContinuationBytesLeft = 3;
					}
					else if ((c & 0xFC) == 0xF8)
					{
						charVal = (uint32_t)(c & 0x03);
						numContinuationBytesLeft = 4;
					}
					else if ((c & 0xFE) == 0xFC)
					{
						charVal = (uint32_t)(c & 0x01);
						numContinuationBytesLeft = 5;
					}
				}
			}
			else if ((c & 0xC0) == 0x80)
			{
				charVal = (charVal << 6) | (c & 0x3F);
				--numContinuationBytesLeft;
				if (numContinuationBytesLeft == 0)
				{
					const char* _ecv_array trtab;
					switch(charVal)
					{
					case 0x0300:	// grave accent
						trtab = trGrave;
						break;
					case 0x0301:	// acute accent
						trtab = trAcute;
						break;
					case 0x0302:	// circumflex
						trtab = trCircumflex;
						break;
					case 0x0303:	// tilde
						trtab = trTilde;
						break;
					case 0x0306:	// breve
						trtab = trBreve;
						break;
					case 0x0308:	// umlaut
						trtab = trUmlaut;
						break;
					case 0x030A:	// small circle
						trtab = trCircle;
						break;
					case 0x030C:	// caron
						trtab = trCaron;
						break;
					case 0x0327:	// cedilla
						trtab = trCedilla;
						break;
					default:
						trtab = nullptr;
						break;
					}

					// If it is a diacritical mark that we handle, try to combine it with the previous character.
					// The diacritical marks are in the range 03xx so they are encoded as 2 UTF8 bytes.
					if (trtab != nullptr && i > 2)
					{
						const char c2 = fieldVal[i - 3];
						while (*trtab != 0 && *trtab != c2)
						{
							trtab += 2;
						}
						if (*trtab != 0)
						{
							// Get the translated character and encode it as 2 UTF8 bytes
							uint16_t c3 = (uint16_t)(uint8_t)trtab[1];
							if (c3 < 0x80)
							{
								c3 |= 0x0100;
							}
							fieldVal[i - 3] = (c3 >> 6) | 0xC0;
							fieldVal[i - 2] = (c3 & 0x3F) | 0x80;
							fieldVal.Erase(i - 1);
							--i;
						}
					}
				}
			}
			else
			{
				// Bad UTF8 state
				numContinuationBytesLeft = 0;
			}
		}
	}

	// Check whether the incoming character signals the end of the value. If it does, process it and return true.
	static bool CheckValueCompleted(char c, bool doProcess)
	{
#if DEBUG
		MessageLog::AppendMessage("CheckValueCompleted");
#endif
		switch(c)
		{
		case ',':
			if (doProcess)
			{
				ProcessField();
			}
			if (InArray())
			{
				++arrayIndices[arrayDepth - 1];
				fieldVal.Clear();
				state = jsVal;
			}
			else
			{
				RemoveLastId();
				state = jsExpectId;
			}
			return true;

		case ']':
			if (InArray())
			{
				if (doProcess)
				{
					ProcessField();
				}
				++arrayIndices[arrayDepth - 1];
				EndArray();
				state = jsEndVal;
			}
			else
			{
				state = jsError;
#if DEBUG
				MessageLog::AppendMessage("jsError: CheckValueCompleted: ]");
#endif
			}
			return true;

		case '}':
			if (InArray())
			{
				state = jsError;
#if DEBUG
				MessageLog::AppendMessage("jsError: CheckValueCompleted: }");
#endif
			}
			else
			{
				if (doProcess)
				{
					ProcessField();
				}
				RemoveLastId();
				if (fieldId.strlen() == 0)
				{
					EndReceivedMessage();
					state = jsBegin;
				}
				else
				{
					RemoveLastIdChar();
					state = jsEndVal;
				}
			}
			return true;

		default:
			return false;
		}
	}

	// This is the JSON parser state machine
	void CheckInput()
	{
		while (nextIn != nextOut)
		{
			char c = rxBuffer[nextOut];
			nextOut = (nextOut + 1) % rxBufsize;
			if (c == '\n')
			{
				if (state == jsError)
				{
#if DEBUG
						MessageLog::AppendMessage("ParserErrorEncountered");
#endif
					ParserErrorEncountered(fieldId.c_str(), fieldVal.c_str(), arrayIndices); // Notify the consumer that we ran into an error
				}
				state = jsBegin;		// abandon current parse (if any) and start again
			}
			else
			{
				switch(state)
				{
				case jsBegin:			// initial state, expecting '{'
					if (c == '{')
					{
						StartReceivedMessage();
						state = jsExpectId;
						fieldVal.Clear();
						fieldId.Clear();
						arrayDepth = 0;
					}
					break;

				case jsExpectId:		// expecting a quoted ID
					switch (c)
					{
					case ' ':
						break;
					case '"':
						state = jsId;
						break;
					case '}':			// empty object, or extra comma at end of field list
						RemoveLastId();
						if (fieldId.strlen() == 0)
						{
							EndReceivedMessage();
							state = jsBegin;
						}
						else
						{
							RemoveLastIdChar();
							state = jsEndVal;
						}
						break;
					default:
						state = jsError;
#if DEBUG
						MessageLog::AppendMessage("jsError: jsExpectId");
#endif
						break;
					}
					break;

				case jsId:				// expecting an identifier, or in the middle of one
					switch (c)
					{
					case '"':
						state = jsHadId;
						break;
					default:
						if (c < ' ')
						{
							state = jsError;
#if DEBUG
							MessageLog::AppendMessage("jsError: jsId 1");
#endif
						}
						else if (c != ':' && c != '^')
						{
							if (fieldId.cat(c))
							{
								state = jsError;
#if DEBUG
								MessageLog::AppendMessage("jsError: jsId 2");
#endif
							}
						}
						break;
					}
					break;

				case jsHadId:			// had a quoted identifier, expecting ':'
					switch(c)
					{
					case ':':
						state = jsVal;
						break;
					case ' ':
						break;
					default:
						state = jsError;
#if DEBUG
						MessageLog::AppendMessage("jsError: jsHadId");
#endif
						break;
					}
					break;

				case jsVal:				// had ':' or ':[', expecting value
					switch(c)
					{
					case ' ':
						break;
					case '"':
						fieldVal.Clear();
						state = jsStringVal;
						break;
					case '[':
						if (arrayDepth < MaxArrayNesting && !fieldId.cat('^'))
						{
							arrayIndices[arrayDepth] = 0;		// start an array
							++arrayDepth;
						}
						else
						{
							state = jsError;
#if DEBUG
							MessageLog::AppendMessage("jsError: [");
#endif
						}
						break;
					case ']':
						if (InArray())
						{
							EndArray();			// empty array
							state = jsEndVal;
						}
						else
						{
							state = jsError;	// ']' received without a matching '[' first
#if DEBUG
							MessageLog::AppendMessage("jsError: ]");
#endif
						}
						break;
					case '-':
						fieldVal.Clear();
						fieldVal.cat(c);
						state = jsNegIntVal;
						break;
					case '{':					// start of a nested object
						state = (!fieldId.cat(':')) ? jsExpectId : jsError;
#if DEBUG
						if (state == jsError)
						{
							MessageLog::AppendMessage("jsError: {");
						}
#endif
						break;
					default:
						if (c >= '0' && c <= '9')
						{
							fieldVal.Clear();
							fieldVal.cat(c);	// must succeed because we just cleared fieldVal
							state = jsIntVal;
						}
						else if (c >= 'a' && c <= 'z')
						{
							fieldVal.Clear();
							fieldVal.cat(c);	// must succeed because we just cleared fieldVal
							state = jsCharsVal;
						}
						else
						{
							state = jsError;
#if DEBUG
							MessageLog::AppendMessage("jsError: jsVal default");
#endif
						}
					}
					break;

				case jsStringVal:		// just had '"' and expecting a string value
					switch (c)
					{
					case '"':
						ConvertUnicode();
						ProcessField();
						state = jsEndVal;
						break;
					case '\\':
						state = jsStringEscape;
						break;
					default:
						if (c < ' ')
						{
							state = jsError;
#if DEBUG
							MessageLog::AppendMessage("jsError: jsStringVal");
#endif
						}
						else
						{
							fieldVal.cat(c);	// ignore any error so that long string parameters just get truncated
						}
						break;
					}
					break;

				case jsStringEscape:	// just had backslash in a string
					if (!fieldVal.IsFull())
					{
						switch (c)
						{
						case '"':
						case '\\':
						case '/':
							if (fieldVal.cat(c))
							{
								state = jsError;
#if DEBUG
								MessageLog::AppendMessage("jsError: jsStringEscape 1");
#endif
							}
							break;
						case 'n':
						case 't':
							if (fieldVal.cat(' '))		// replace newline and tab by space
							{
								state = jsError;
#if DEBUG
								MessageLog::AppendMessage("jsError: jsStringEscape 2");
#endif
							}
							break;
						case 'b':
						case 'f':
						case 'r':
						default:
							break;
						}
					}
					state = jsStringVal;
					break;

				case jsNegIntVal:		// had '-' so expecting a integer value
					state = (c >= '0' && c <= '9' && !fieldVal.cat(c)) ? jsIntVal : jsError;
#if DEBUG
						if (state == jsError)
						{
							MessageLog::AppendMessage("jsError: jsNegIntVal");
						}
#endif
					break;

				case jsIntVal:			// receiving an integer value
					if (CheckValueCompleted(c, true))
					{
						break;
					}

					if (c == '.')
					{
						state = (!fieldVal.cat(c)) ? jsFracVal : jsError;
#if DEBUG
						if (state == jsError)
						{
							MessageLog::AppendMessage("jsError: jsIntVal");
						}
#endif
					}
					else if (!(c >= '0' && c <= '9' && !fieldVal.cat(c)))
					{
						state = jsError;
#if DEBUG
						MessageLog::AppendMessage("jsError: jsIntVal");
#endif
					}
					break;

				case jsFracVal:			// receiving a fractional value
					if (CheckValueCompleted(c, true))
					{
						break;
					}

					if (!(c >= '0' && c <= '9' && !fieldVal.cat(c)))
					{
						state = jsError;
#if DEBUG
						MessageLog::AppendMessage("jsError: jsFracVal");
#endif
					}
					break;

				case jsCharsVal:
					if (CheckValueCompleted(c, true))
					{
						break;
					}

					if (!(c >= 'a' && c <= 'z' && !fieldVal.cat(c)))
					{
						state = jsError;
#if DEBUG
						MessageLog::AppendMessage("jsError: jsCharsVal");
#endif
					}
					break;

				case jsEndVal:			// had the end of a string or array value, expecting comma or ] or }
					if (CheckValueCompleted(c, false))
					{
						break;
					}

					state = jsError;
#if DEBUG
					MessageLog::AppendMessage("jsError: jsEndVal");
#endif
					break;

				case jsError:
					// Ignore all characters. State will be reset to jsBegin at the start of this function when we receive a newline.
					break;
				}
			}
		}
	}

	// Called by the ISR to store a received character.
	// If the buffer is full, we wait for the next end-of-line.
	void receiveChar(char c)
	{
		if (c == '\n')
		{
			inError = false;
		}
		if (!inError)
		{
			size_t temp = (nextIn + 1) % rxBufsize;
			if (temp == nextOut)
			{
				inError = true;
			}
			else
			{
				rxBuffer[nextIn] = c;
				nextIn = temp;
			}
		}
	}

	// Called by the ISR to signify an error. We wait for the next end of line.
	void receiveError()
	{
		inError = true;
	}
}

extern "C" {

#if SAM4S
	void UART0_Handler()
#else
	void UART1_Handler()
#endif
	{
		uint32_t status = UARTn->UART_SR;

		// Did we receive data ?
		if ((status & UART_SR_RXRDY) == UART_SR_RXRDY)
		{
			SerialIo::receiveChar(UARTn->UART_RHR);
		}

		// Acknowledge errors
		if (status & (UART_SR_OVRE | UART_SR_FRAME))
		{
			UARTn->UART_CR |= UART_CR_RSTSTA;
			SerialIo::receiveError();
		}
	}

};

// End
