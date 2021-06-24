#ifndef FLASH_DATA
#define FLASH_DATA 1

#include <cstdint>

#include "Hardware/Backlight.hpp"
#include "General/SimpleMath.h"
#include "UI/Display.hpp"

enum class DisplayDimmerType : uint8_t
{
	never = 0,				// never dim the display
	onIdle, 				// only display when printer status is idle
	always,					// default - always dim
	NumTypes
};

enum class HeaterCombineType : uint8_t
{
	notCombined = 0,
	combined,
	NumTypes
};

struct FlashData;

extern FlashData nvData, savedNvData;


struct FlashData
{
	// The magic value should be changed whenever the layout of the NVRAM changes
	// We now use a different magic value for each display size, to force the "touch the spot" screen to be displayed when you change the display size
	static const uint32_t magicVal = 0x3AB64A40 + DISPLAY_TYPE;
	static const uint32_t muggleVal = 0xFFFFFFFF;

	alignas(4) uint32_t magic;
	uint32_t baudRate;
	uint16_t xmin;
	uint16_t xmax;
	uint16_t ymin;
	uint16_t ymax;
	DisplayOrientation lcdOrientation;
	DisplayOrientation touchOrientation;
	uint8_t touchVolume;
	uint8_t language;
	uint8_t colourScheme;
	uint8_t brightness;
	DisplayDimmerType displayDimmerType;
	uint8_t infoTimeout;
	uint32_t screensaverTimeout;
	uint8_t babystepAmountIndex;
	uint16_t feedrate;
	HeaterCombineType heaterCombineType;
	alignas(4) char dummy;								// must be at a multiple of 4 bytes from the start because flash is read/written in whole dwords

	FlashData() : magic(muggleVal) { SetDefaults(); }
	bool operator==(const FlashData& other);
	bool operator!=(const FlashData& other) { return !operator==(other); }
	bool IsValid() const;
	void SetInvalid() { magic = muggleVal; }
	void SetDefaults();
	void Load();
	void Save() const;

	bool IsSaveNeeded();

	bool SetColourScheme(uint8_t newColours);
	void SetInfoTimeout(uint8_t newInfoTimeout) { nvData.infoTimeout = newInfoTimeout; }
	bool SetLanguage(uint8_t newLanguage);

	void SetDisplayDimmerType(DisplayDimmerType newType) { nvData.displayDimmerType = newType; }
	DisplayDimmerType GetDisplayDimmerType() { return nvData.displayDimmerType; }

	void SetVolume(uint8_t newVolume) { nvData.touchVolume = newVolume; }
	uint32_t GetVolume() { return nvData.touchVolume; }

	void SetScreensaverTimeout(uint32_t screensaverTimeout) { nvData.screensaverTimeout = screensaverTimeout; }
	uint32_t GetScreensaverTimeout() { return nvData.screensaverTimeout; }

	void SetBabystepAmountIndex(uint8_t babystepAmountIndex) { nvData.babystepAmountIndex = babystepAmountIndex; }
	uint8_t GetBabystepAmountIndex() { return nvData.babystepAmountIndex; }

	void SetFeedrate(uint16_t feedrate) { nvData.feedrate = feedrate; }
	uint16_t GetFeedrate() { return nvData.feedrate; }

	void SetHeaterCombineType(HeaterCombineType combine) { nvData.heaterCombineType = combine; }
	HeaterCombineType GetHeaterCombineType() { return nvData.heaterCombineType; }

	void SetBaudRate(uint32_t rate) { nvData.baudRate = rate; }
	uint32_t GetBaudRate() { return nvData.baudRate; }

	void SetBrightness(uint32_t percent) { nvData.brightness =
		constrain<int>(percent, Backlight::MinBrightness, Backlight::MaxBrightness); }
	int GetBrightness() { return (int)nvData.brightness; }


};

#if SAM4S
// FlashData must fit in user signature flash area
static_assert(sizeof(FlashData) <= 512, "Flash data too large");
#else
// FlashData must fit in the area we have reserved
static_assert(sizeof(FlashData) <= FLASH_DATA_LENGTH, "Flash data too large");
#endif

#endif /* ifndef FLASH_DATA */
