#ifndef DEBUG_HPP
#define DEBUG_HPP 1

// enable debugging output globally set DEBUG here
// enable debugging locally set DEBUG before including this header
//#define DEBUG	(0) // 0: off, 1: MessageLog, 2: Uart

#if (DEBUG == 1)
#include <UI/MessageLog.hpp>

#define dbg(fmt, args...)		do { MessageLog::AppendMessageF(MessageLog::LogLevel::Verbose, "%s(%d): " fmt , __FUNCTION__, __LINE__, ##args); } while(0)

#elif (DEBUG == 2)
#include "Hardware/SerialIo.hpp"

#define dbg(fmt, args...)		do { SerialIo::Dbg("%s(%d): " fmt, __FUNCTION__, __LINE__, ##args); } while(0)

#else
#define dbg(fmt, args...)		do {} while(0)

#endif


#endif /* ifndef DEBUG_HPP */
