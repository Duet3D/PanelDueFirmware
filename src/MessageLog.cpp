/*
 * MessageLog.cpp
 *
 * Created: 15/11/2015 10:54:42
 *  Author: David
 */ 

#include "ecv.h"
#include "asf.h"
#include "Configuration.hpp"
#include "Library/Vector.hpp"
#include "MessageLog.hpp"
#include "Fields.hpp"
#include "Hardware/SysTick.hpp"
#include "Library/Misc.hpp"

namespace MessageLog
{
	const unsigned int maxMessageChars = 80;

	struct Message
	{
		static const size_t rttLen = 5;					// number of chars we print for the message age
		uint32_t receivedTime;
		char receivedTimeText[rttLen];					// 5 characters plus null terminator
		char msg[maxMessageChars + 1];
	};

	static Message messages[numMessageRows + 1];		// one extra slot for receiving new messages into
	static unsigned int messageStartRow = 0;			// the row number at the top
	static unsigned int newMessageStartRow = 0;			// the row number that we put a new message in

	void Init()
	{
		// Clear the message log
		for (size_t i = 0; i <= numMessageRows; ++i)	// note we have numMessageRows+1 message slots
		{
			messages[i].receivedTime = 0;
			messages[i].msg[0] = 0;
		}
		
		UpdateMessages(true);
	}
	
	// Update the messages on the message tab. If 'all' is true we do the times and the text, else we just do the times.
	void UpdateMessages(bool all)
	{
		size_t index = messageStartRow;
		for (size_t i = 0; i < numMessageRows; ++i)
		{
			Message *m = &messages[index];
			uint32_t tim = m->receivedTime;
			char* p = m->receivedTimeText;
			if (tim == 0)
			{
				p[0] = 0;
			}
			else
			{
				uint32_t age = (SystemTick::GetTickCount() - tim)/1000;	// age of message in seconds
				if (age < 10 * 60)
				{
					snprintf(p, Message::rttLen, "%lum%02lu", age/60, age%60);
				}
				else
				{
					age /= 60;		// convert to minutes
					if (age < 60)
					{
						snprintf(p, Message::rttLen, "%lum", age);
					}
					else if (age < 10 * 60)
					{
						snprintf(p, Message::rttLen, "%luh%02lu", age/60, age%60);
					}
					else
					{
						age /= 60;	// convert to hours
						if (age < 10)
						{
							snprintf(p, Message::rttLen, "%luh", age);
						}
						else if (age < 24 + 10)
						{
							snprintf(p, Message::rttLen, "%lud%02lu", age/24, age%24);
						}
						else
						{
							snprintf(p, Message::rttLen, "%lud", age/24);
						}
					}
				}
			}
			messageTimeFields[i]->SetValue(p);

			if (all)
			{
				messageTextFields[i]->SetValue(m->msg);
			}
			index = (index + 1) % (numMessageRows + 1);
		}
	}

	// Add a message to the end of the list. It will be just off the visible part until we scroll it in.
	void AppendMessage(const char* array data)
	{
		// Skip any leading spaces, we don't have room on the display to waste
		while (*data == ' ')
		{
			++data;
		}
		
		// Discard empty messages
		if (*data != 0)
		{
			bool split;
			unsigned int numLines = 0;
			do
			{
				++numLines;
				size_t msgRow = (messageStartRow + numLines + numMessageRows - 1) % (numMessageRows + 1);
				size_t splitPoint;
			
				// See if the rest of the message will fit on one line
				if (numLines == numMessageRows)
				{
					split = false;		// if we have printed the maximum number of rows, don't split any more, just truncate
				}
				else
				{
					splitPoint = FindSplitPoint(data, maxMessageChars, messageTextWidth);
					split = data[splitPoint] != '\0';
				}
			
				if (split)
				{
					safeStrncpy(messages[msgRow].msg, data, splitPoint + 1);
					data += splitPoint;
					if (data[0] == ' ')
					{
						++data;			// if we split just before a space, don't show the space
					}
				}
				else
				{
					safeStrncpy(messages[msgRow].msg, data, maxMessageChars + 1);
				}
			
				messages[msgRow].receivedTime = (numLines == 1) ? SystemTick::GetTickCount() : 0;
			} while (split && data[0] != '\0');

			newMessageStartRow = (messageStartRow + numLines) % (numMessageRows + 1);
		}
	}

	// If there is a new message, scroll it in
	void DisplayNewMessage()
	{
		if (newMessageStartRow != messageStartRow)
		{
			messageStartRow = newMessageStartRow;
			UpdateMessages(true);
		}
	}
	
	// This is called when we receive a new response from the host, which may or may not include a new message for the log
	void BeginNewMessage()
	{
		newMessageStartRow = messageStartRow;
	}

	// Find where we need to split a text string so that it will fit in  a field
	size_t FindSplitPoint(const char * array s, size_t maxChars, PixelNumber width)
	{
		const size_t remLength = strlen(s);
		maxChars = min<size_t>(maxChars, maxMessageChars);
		if (remLength > maxChars || DisplayField::GetTextWidth(s, width + 1) > width)
		{
			// We need to split the line, so find out where
			size_t low = 0, high = min<size_t>(remLength, maxChars + 1);
			while (low + 1 < high)
			{
				size_t mid = (low + high)/2;
				char buf[maxMessageChars + 1];
				safeStrncpy(buf, s, mid + 1);
				if (DisplayField::GetTextWidth(buf, messageTextWidth + 1) <= messageTextWidth)
				{
					low = mid;
				}
				else
				{
					high = mid;
				}
			}
				
			// The first 'low' characters fit, but no more.
			// Look for a space or other character where we can split the line neatly
			size_t splitPoint = low;
			if (s[splitPoint] != ' ')
			{
				while (splitPoint > 0)
				{
					if (s[splitPoint - 1] == ' ' || s[splitPoint - 1] == ',')
					{
						// We can split after space or comma
						break;
					}
					if ((low - splitPoint) * 5 > low)
					{
						// If there is no good split point within 1/5 of ther most that will fit, split anyway
						splitPoint = low;
						break;
					}
					--splitPoint;
				}
			}
			return splitPoint;
		}
		return remLength;
	}

}			// end namespace

// End
