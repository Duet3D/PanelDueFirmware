#ifndef UI_ALERT_HPP
#define UI_ALERT_HPP 1

#include <cstdint>
#include <cstddef>
#include <float.h>
#include "General/Bitmap.h"
#include "General/SimpleMath.h"
#include "General/String.h"
#include "General/Vector.hpp"

const size_t alertTextLength = 165;			// maximum characters in the alert text
const size_t alertTitleLength = 50;			// maximum characters in the alert title

struct Alert
{
	enum Mode : int32_t {
		None = -1,
		Info = 0,
		InfoClose = 1,
		InfoConfirm = 2,
		ConfirmCancel = 3,
		Choices = 4,
		NumberInt = 5,
		NumberFloat = 6,
		Text = 7,
		Max
	} mode;
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

	bool cancelButton;
	String<32> choices[10];
	size_t choices_count;

	struct Limits {
		struct {
			int32_t min;
			int32_t max;
			int32_t valueDefault;
		} numberInt;
		struct {
			float min;
			float max;
			float valueDefault;
		} numberFloat;
		struct {
			int32_t min;
			int32_t max;
			String<32> valueDefault;
		} text;
	} limits;

	Alert() : mode(Mode::Info), seq(0), controls(0), timeout(0.0) { Reset(); }

	bool AllFlagsSet() const { return flags.GetRaw() == GotAll; }
	void Reset()
	{
		mode = Mode::None;
		flags.Clear();
		for (size_t i = 0; i < ARRAY_SIZE(choices); i++)
		{
			choices[i].Clear();
		}
		limits.numberInt.min = 0;
		limits.numberInt.max = INT32_MAX;
		limits.numberFloat.min = 0;
		limits.numberFloat.max = FLT_MAX;
		limits.text.min = 1;
		limits.text.max = 10;
	}
};

#endif /* ifndef UI_ALERT_HPP */
