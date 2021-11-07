#ifndef UI_ALERT_HPP
#define UI_ALERT_HPP 1

#include <cstdint>
#include "General/Bitmap.h"
#include "General/String.h"

const size_t alertTextLength = 165;			// maximum characters in the alert text
const size_t alertTitleLength = 50;			// maximum characters in the alert title

class Alert
{
private:
public:
	int32_t mode;
	uint32_t seq;
	uint32_t controls;
	float timeout;
	Bitmap<uint8_t> flags;
	String<50> title;
	String<alertTextLength> text;

	static constexpr uint8_t GotMode = 0;
	static constexpr uint8_t GotSeq = 1;
	static constexpr uint8_t GotTimeout = 2;
	static constexpr uint8_t GotTitle = 3;
	static constexpr uint8_t GotText = 4;
	static constexpr uint8_t GotControls = 5;
	static constexpr uint8_t GotAll =
			(1 << GotMode)
			| (1 << GotSeq)
			| (1 << GotTimeout)
			| (1 << GotTitle)
			| (1 << GotText)
			| (1 << GotControls);

	Alert() : mode(0), seq(0), controls(0), timeout(0.0) { flags.Clear(); }

	bool AllFlagsSet() const { return flags.GetRaw() == GotAll; }
};

#endif /* ifndef UI_ALERT_HPP */
