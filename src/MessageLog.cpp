/*
 * MessageLog.cpp
 *
 * Created: 15/11/2015 10:54:42
 *  Author: David
 */ 

#include "MessageLog.hpp"
#include "asf.h"
#include "UserInterfaceConstants.hpp"
#include "UserInterface.hpp"
#include "Hardware/SysTick.hpp"
#include "Library/Misc.hpp"
#include "General/String.h"
#include "General/SafeVsnprintf.h"

namespace MessageLog
{
	const unsigned int MaxCharsPerRow = 80;

	const unsigned int MaxCharsPerMessage = 300;		// variable fieldVal in module SerialIO must also be large enough for this
	const unsigned int MaxNewMessageLines = 6;

	struct Message
	{
		static const size_t rttLen = 5;					// number of chars we print for the message age
		uint32_t receivedTime;
		char receivedTimeText[rttLen];					// 5 characters plus null terminator
		char msg[MaxCharsPerRow + 1];
	};

	static String<MaxCharsPerMessage> newMessage;		// buffer for receiving a new message into
	static Message messages[numMessageRows];
	static unsigned int messageStartRow = 0;			// the row number at the top

	void Init()
	{
		// Clear the message log
		for (size_t i = 0; i < numMessageRows; ++i)	// note we have numMessageRows+1 message slots
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
					SafeSnprintf(p, Message::rttLen, "%lum%02lu", age/60, age%60);
				}
				else
				{
					age /= 60;		// convert to minutes
					if (age < 60)
					{
						SafeSnprintf(p, Message::rttLen, "%lum", age);
					}
					else if (age < 10 * 60)
					{
						SafeSnprintf(p, Message::rttLen, "%luh%02lu", age/60, age%60);
					}
					else
					{
						age /= 60;	// convert to hours
						if (age < 10)
						{
							SafeSnprintf(p, Message::rttLen, "%luh", age);
						}
						else if (age < 24 + 10)
						{
							SafeSnprintf(p, Message::rttLen, "%lud%02lu", age/24, age%24);
						}
						else
						{
							SafeSnprintf(p, Message::rttLen, "%lud", age/24);
						}
					}
				}
			}
			messageTimeFields[i]->SetValue(p, true);

			if (all)
			{
				messageTextFields[i]->SetValue(m->msg);
			}
			index = (index + 1) % numMessageRows;
		}
	}

	// Add a message to the end of the list
	// Call this only with a non empty message having no leading whitespace
	void AppendMessage(const char* _ecv_array data)
	{
		bool split;
		unsigned int numLines = 0;
		do
		{
			size_t msgRow = (messageStartRow + numLines) % numMessageRows;
			++numLines;
			size_t splitPoint;

			// See if the rest of the message will fit on one line
			if (numLines == numMessageRows)
			{
				split = false;		// if we have printed the maximum number of rows (unlikely), don't split any more, just truncate
			}
			else
			{
				splitPoint = FindSplitPoint(data, MaxCharsPerRow, messageTextWidth);
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
				safeStrncpy(messages[msgRow].msg, data, MaxCharsPerRow + 1);
			}

			messages[msgRow].receivedTime = (numLines == 1) ? SystemTick::GetTickCount() : 0;
		} while (split && data[0] != '\0');

		messageStartRow = (messageStartRow + numLines) % numMessageRows;
		UpdateMessages(true);
	}

	void AppendMessage(size_t maxLen, const char* format, ...)
	{
		char msg[maxLen];
		va_list vargs;
		va_start(vargs, format);
		SafeVsnprintf(msg, maxLen, format, vargs);
		va_end(vargs);
		AppendMessage(msg);
	}

	// Save a message for possible display later
	void SaveMessage(const char* data)
	{
		newMessage.copy(data);
	}

	// If there is a new message, scroll it in
	void DisplayNewMessage()
	{
		if (!newMessage.IsEmpty())
		{
			const char * _ecv_array msg = newMessage.c_str();
			// Skip any leading spaces, we don't have room on the display to waste
			while (*msg == ' ' || *msg == '\n')
			{
				++msg;
			}

			// Discard empty messages
			if (*msg != 0)
			{
				AppendMessage(msg);
				UI::NewResponseReceived(msg);
			}
			newMessage.Clear();
		}
	}
	
	// This is called when we receive a new response from the host, which may or may not include a new message for the log
	void BeginNewMessage()
	{
		newMessage.Clear();
	}

	// Find where we need to split a text string so that it will fit in a field
	size_t FindSplitPoint(const char * _ecv_array s, size_t maxChars, PixelNumber width)
	{
		const size_t remLength = strlen(s);
		maxChars = min<size_t>(maxChars, MaxCharsPerRow);
		if (remLength > maxChars || DisplayField::GetTextWidth(s, width + 1) > width)
		{
			// We need to split the line, so find out where
			size_t low = 0, high = min<size_t>(remLength, maxChars + 1);
			while (low + 1 < high)
			{
				const size_t mid = (low + high)/2;
				if (DisplayField::GetTextWidth(s, messageTextWidth + 1, mid) <= messageTextWidth)
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
						// If there is no good split point within 1/5 of the most that will fit, split anyway
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
