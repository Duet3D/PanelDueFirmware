/*
 * MessageLog.h
 *
 * Created: 15/11/2015 10:55:02
 *  Author: David
 */ 


#ifndef MESSAGELOG_H_
#define MESSAGELOG_H_

#include <cstddef>
#include <UI/DisplaySize.hpp>
#include "ecv.h"
#undef array
#undef result
#undef value

namespace MessageLog
{
	void Init();

	// Update the messages on the message tab.
	void UpdateMessages();

	// Add a message to the end of the list immediately
	void AppendMessage(const char* data);

	// Add a message via printf to the end of the list immediately (mainly for debugging)
	void AppendMessageF(const char* format, ...) __attribute__((format (printf, 1, 0)));

	// Save a message for possible display later
	void SaveMessage(const char* data);

	// If we saved a message, display it
	void DisplayNewMessage();
	
	// This is called when we receive a new response from the host, which may or may not include a new message for the log
	void BeginNewMessage();
	
	// Find where we need to split a text string so that it will fit in  a field
	size_t FindSplitPoint(const char * _ecv_array s, size_t maxChars, PixelNumber width);
}

#endif /* MESSAGELOG_H_ */
