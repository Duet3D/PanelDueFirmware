/*
 * MessageLog.h
 *
 * Created: 15/11/2015 10:55:02
 *  Author: David
 */ 


#ifndef MESSAGELOG_H_
#define MESSAGELOG_H_

#include "Display.hpp"

namespace MessageLog
{
	void Init();

	// Update the messages on the message tab. If 'all' is true we do the times and the text, else we just do the times.
	void UpdateMessages(bool all);

	// Add a message to the end of the list. It will be just off the visible part until we scroll it in.
	void AppendMessage(const char* data);

	// If there is a new message, scroll it in
	void DisplayNewMessage();
	
	// This is called when we receive a new response from the host, which may or may not include a new message for the log
	void BeginNewMessage();
	
	// Find where we need to split a text string so that it will fit in  a field
	size_t FindSplitPoint(const char * array s, size_t maxChars, PixelNumber width);
}

#endif /* MESSAGELOG_H_ */