/*
 * SerialIo.cpp
 *
 * Created: 09/11/2014 09:20:26
 *  Author: David
 */

#include "SerialIo.hpp"
#include <Hardware/SysTick.hpp>
#include "asf.h"
#include "PanelDue.hpp"
#include <General/CRC16.h>
#include <General/String.h>
#include <General/SafeVsnprintf.h>

#define DEBUG 0
#include "Debug.hpp"

const size_t MaxArrayNesting = 6;

#if SAM4S
# define UARTn	UART0
#else
# define UARTn	UART1
#endif


namespace SerialIo
{
	static unsigned int lineNumber = 0;
	uint16_t numChars = 0;
	uint8_t checksum = 0;
	CRC16 crc;
	volatile uint32_t timeLastCharacterReceived = 0;

	enum CheckType {
		None,
		Simple,
		CRC16
	} check;


	static struct SerialIoCbs *cbs = nullptr;
	static int serialIoErrors = 0;

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
	void Init(uint32_t baudRate, struct SerialIoCbs *callbacks)
	{
		cbs = callbacks;

		check = CheckType::CRC16;

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

	void SetBaudRate(uint32_t baudRate)
	{
		Init(baudRate, cbs);
	}

	void SetCRC16(bool enable)
	{
		if (enable)
		{
			check = CheckType::CRC16;
		}
		else
		{
			check = CheckType::Simple;
		}
	}

	// Send a character to the 3D printer.
	// A typical command string is only about 12 characters long, which at 115200 baud takes just over 1ms to send.
	// So there is no particular reason to use interrupts, and by so doing so we avoid having to handle buffer full situations.
	static void RawSendChar(char c)
	{
		while(uart_write(UARTn, c) != 0) { }
	}

	static void SendCharAndChecksum(char c)
	{
		switch (check)
		{
		case CheckType::None:
			break;
		case CheckType::Simple:
			checksum ^= c;
			break;
		case CheckType::CRC16:
			crc.Update(c);
			break;
		}
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
				switch (check)
				{
				case CheckType::None:
					break;
				case CheckType::Simple:
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
					break;
				case CheckType::CRC16:
					{
						uprintf([](char c) noexcept -> bool {
								if (c != 0)
								{
								RawSendChar(c);
								}
								return true;
							}, "*%05d", crc.Get());
						crc.Reset(0);
					}
					break;
				}
			}
			RawSendChar(c);
			numChars = 0;
		}
		else
		{
			if (numChars == 0)
			{
				checksum = 0;
				crc.Reset(0);
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
		int ret = vuprintf([](char c) noexcept -> bool {
			if (c != 0)
			{
				SendChar(c);
			}
			return true;
		}, fmt, vargs);
		va_end(vargs);

		return ret;
	}

	size_t Dbg(const char *fmt, ...)
	{
		char buffer[128];
		int ret;
		int ret2;
		va_list vargs;

		ret = SafeSnprintf(buffer, sizeof(buffer), ";dbg %4lu ", SystemTick::GetTickCount() / 1000);
		if (ret < 0)
			return 0;

		va_start(vargs, fmt);
		ret2 = SafeVsnprintf(&buffer[ret], sizeof(buffer) - ret, fmt, vargs);
		va_end(vargs);
		if (ret2 < 0)
			return 0;

		ret += ret2;
		for (int i = 0; i < ret; i++) {
			while(uart_write(UARTn, buffer[i]))
				;;
		}

		return ret;
	}

	void SendFilename(const char * _ecv_array dir, const char * _ecv_array name)
	{
		const char* quote = GetFirmwareFeatures().IsBitSet(quoteFilenames) ? "\"" : "";
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
		jsExpValSign,		// about to receive an exponent, possible sign coming up
		jsExpValFirstDigit,	// expecting the first digit of an exponent
		jsExpValDigits,		// expecting remaining digits of an exponent
		jsError				// something went wrong
	};

	JsonState state = jsBegin;
	JsonState lastState = jsBegin;

	// fieldId is the name of the field being received. A '^' character indicates the position of an _ecv_array index, and a ':' character indicates a field separator.
	String<150> fieldId;
	String<1028> fieldVal;
	size_t arrayIndices[MaxArrayNesting];
	size_t arrayDepth = 0;

	static void RemoveLastId()
	{
		//dbg("%s, len: %d", fieldId.c_str(), fieldId.strlen());
		size_t index = fieldId.strlen();
		while (index != 0 && fieldId[index - 1] != '^' && fieldId[index - 1] != ':')
		{
			--index;
		}
		fieldId.Truncate(index);

		//dbg("RemoveLastId: %s, len: %d", fieldId.c_str(), fieldId.strlen());
	}

	static void RemoveLastIdChar()
	{
		//dbg();

		if (fieldId.strlen() != 0)
		{
			fieldId.Truncate(fieldId.strlen() - 1);
		}
	}

	static bool InArray()
	{
		//dbg();

		return fieldId.strlen() > 0 && fieldId[fieldId.strlen() - 1] == '^';
	}

	static void ProcessField()
	{

		if (state == jsCharsVal)
		{
			if (fieldVal.Equals("null"))
			{
				fieldVal.Clear();				// so that we can distinguish null from an empty string
			}
		}
		if (cbs && cbs->ProcessReceivedValue)
		{
			dbg("%s: %s", fieldId.c_str(), fieldVal.c_str());
			cbs->ProcessReceivedValue(fieldId.GetRef(), fieldVal.c_str(), arrayIndices);
		}
		fieldVal.Clear();
	}

	static void EndArrayElement(const char *id, size_t index)
	{
		dbg("id %s index %lu\r\n", id, index);

		if (cbs && cbs->ProcessArrayElementEnd)
		{
			cbs->ProcessArrayElementEnd(id, index);
		}
	}

	static void EndArray()
	{
		//dbg();

		if (cbs && cbs->ProcessArrayEnd)
		{
			cbs->ProcessArrayEnd(fieldId.c_str(), arrayIndices);
		}
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
		//dbg();

		switch(c)
		{
		case ',':
			if (doProcess)
			{
				ProcessField();
			}
			if (InArray())
			{
				EndArrayElement(fieldId.c_str(), arrayIndices[arrayDepth - 1]);

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
				EndArrayElement(fieldId.c_str(), arrayIndices[arrayDepth - 1]);

				++arrayIndices[arrayDepth - 1];
				EndArray();
				state = jsEndVal;
			}
			else
			{
				state = jsError;
				dbg("jsError: CheckValueCompleted: ]");
			}
			return true;

		case '}':
			if (InArray())
			{
				state = jsError;
				dbg("jsError: CheckValueCompleted: }");
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
					serialIoErrors = 0;

					if (cbs && cbs->EndReceivedMessage)
					{
						cbs->EndReceivedMessage();
					}
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
					dbg("ParserErrorEncountered");
					serialIoErrors++;

					if (cbs && cbs->ParserErrorEncountered)
					{
						cbs->ParserErrorEncountered(lastState, fieldId.c_str(), serialIoErrors); // Notify the consumer that we ran into an error
						lastState = jsBegin;
					}
				}
				state = jsBegin;		// abandon current parse (if any) and start again
			}
			else
			{
				lastState = state;

				switch(state)
				{
				case jsBegin:			// initial state, expecting '{'
					if (c == '{')
					{
						if (cbs && cbs->StartReceivedMessage)
						{
							cbs->StartReceivedMessage();
						}
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
							serialIoErrors = 0;

							if (cbs && cbs->EndReceivedMessage)
							{
								cbs->EndReceivedMessage();
							}
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
						dbg("jsError: jsExpectId");
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
							dbg("jsError: jsId 1");
						}
						else if (c != ':' && c != '^')
						{
							if (fieldId.cat(c))
							{
								state = jsError;
								dbg("jsError: jsId 2");
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
						dbg("jsError: jsHadId");
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
							dbg("jsError: [");
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
							dbg("jsError: ]");
						}
						break;
					case '-':
						fieldVal.Clear();
						fieldVal.cat(c);
						state = jsNegIntVal;
						break;
					case '{':					// start of a nested object
						state = (!fieldId.cat(':')) ? jsExpectId : jsError;

						if (state == jsError)
						{
							dbg("jsError: {");
						}
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
							dbg("jsError: jsVal default");
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
							dbg("jsError: jsStringVal");
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
								dbg("jsError: jsStringEscape 1");
							}
							break;
						case 'n':
						case 't':
							if (fieldVal.cat(' '))		// replace newline and tab by space
							{
								state = jsError;
								dbg("jsError: jsStringEscape 2");
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
					if (state == jsError)
					{
						dbg("jsError: jsNegIntVal");
					}
					break;

				case jsIntVal:			// receiving an integer value
					if (CheckValueCompleted(c, true))
					{
						break;
					}

					if (c == '.')
					{
						state = (!fieldVal.cat(c)) ? jsFracVal : jsError;
						if (state == jsError)
						{
							dbg("jsError: jsIntVal");
						}
					}
					else if (!(c >= '0' && c <= '9' && !fieldVal.cat(c)))
					{
						state = jsError;
						dbg("jsError: jsIntVal");
					}
					break;

				case jsFracVal:			// receiving a fractional value
					if (CheckValueCompleted(c, true))
					{
						break;
					}

					if ((c == 'e' || c == 'E') && !fieldVal.cat(c))
					{
						state = jsExpValSign;
					}
					else if (!(c >= '0' && c <= '9' && !fieldVal.cat(c)))
					{
						state = jsError;
						dbg("jsError: jsFracVal(%c)", c);
					}
					break;

				case jsExpValSign:
					if (c == '-' || c == '+')
					{
						if (fieldVal.cat(c))
						{
							state = jsError;
							dbg("jsError: jsExpValSign(%c)", c);
						}
						else
						{
							state = jsExpValFirstDigit;
						}
						break;
					}

					state = jsExpValFirstDigit;
					// fall through
					// no break (for Eclipse, the previous line is for gcc)
				case jsExpValFirstDigit:
					if (!(c >= '0' && c <= '9' && !fieldVal.cat(c)))
					{
						state = jsError;
						dbg("jsError: jsExpValFirstDigit(%c)", c);
						break;
					}
					state = jsExpValDigits;
					break;

				case jsExpValDigits:
					if (CheckValueCompleted(c, true))
					{
						break;
					}
					if (!(c >= '0' && c <= '9' && !fieldVal.cat(c)))
					{
						state = jsError;
						dbg("jsError: jsExpValDigits(%c)", c);
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
						dbg("jsError: jsCharsVal");
					}
					break;

				case jsEndVal:			// had the end of a string or array value, expecting comma or ] or }
					if (CheckValueCompleted(c, false))
					{
						break;
					}

					state = jsError;
					dbg("jsError: jsEndVal");
					break;

				case jsError:
					// Ignore all characters. State will be reset to jsBegin at the start of this function when we receive a newline.
					break;
				}

#if DEBUG
				if (lastState != state) { dbg("state %d -> %d", lastState, state); }
#endif
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
		timeLastCharacterReceived = SystemTick::GetTickCount();
	}

	// Called by the ISR to signify an error. We wait for the next end of line.
	void receiveError()
	{
		inError = true;
	}

	// Return true if the serial line has been quiet for sufficient time.
	// The purpose of this is to prevent PanelDue from hanging on to RRF output buffers for all of the time, which prevents DWC from retrieving the object model.
	// Call this and check the return before sending a request that has a long response to RRF
	bool SerialLineQuiet()
	{
		const uint32_t loc_timeLastCharacterReceived = timeLastCharacterReceived;		// capture this before we call mills() in case of an interrupt
		return SystemTick::GetTickCount() - loc_timeLastCharacterReceived >= MinimumLineQuietTime;
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
