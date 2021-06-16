#include "FlashData.hpp"

#include "Hardware/Buzzer.hpp"
#include "Hardware/Backlight.hpp"
#include "UI/UserInterface.hpp"

#if SAM4S
#include "flash_efc.h"
#else
#include <Hardware/FlashStorage.hpp>
#endif

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

FlashData nvData, savedNvData;

bool FlashData::IsValid() const
{
	return magic == magicVal
		&& touchVolume <= Buzzer::MaxVolume
		&& brightness >= Backlight::MinBrightness
		&& brightness <= Backlight::MaxBrightness
		&& language < UI::GetNumLanguages()
		&& colourScheme < NumColourSchemes
		&& displayDimmerType < DisplayDimmerType::NumTypes
		&& babystepAmountIndex < ARRAY_SIZE(babystepAmounts)
		&& feedrate > 0
		&& heaterCombineType < HeaterCombineType::NumTypes;
}

bool FlashData::operator==(const FlashData& other)
{
	return magic == other.magic
		&& baudRate == other.baudRate
		&& xmin == other.xmin
		&& xmax == other.xmax
		&& ymin == other.ymin
		&& ymax == other.ymax
		&& lcdOrientation == other.lcdOrientation
		&& touchOrientation == other.touchOrientation
		&& touchVolume == other.touchVolume
		&& language == other.language
		&& colourScheme == other.colourScheme
		&& brightness == other.brightness
		&& displayDimmerType == other.displayDimmerType
		&& infoTimeout == other.infoTimeout
		&& screensaverTimeout == other.screensaverTimeout
		&& babystepAmountIndex == other.babystepAmountIndex
		&& feedrate == other.feedrate
		&& heaterCombineType == other.heaterCombineType;
}

void FlashData::SetDefaults()
{
	baudRate = DefaultBaudRate;
	xmin = 0;
	xmax = DisplayX - 1;
	ymin = 0;
	ymax = DisplayY - 1;
	lcdOrientation = DefaultDisplayOrientAdjust;
	touchOrientation = DefaultTouchOrientAdjust;
	touchVolume = Buzzer::DefaultVolume;
	brightness = Backlight::MaxBrightness;
	language = 0;
	colourScheme = 0;
	displayDimmerType = DisplayDimmerType::always;
	infoTimeout = DefaultInfoTimeout;
	screensaverTimeout = DefaultScreensaverTimeout;
	babystepAmountIndex = DefaultBabystepAmountIndex;
	feedrate = DefaultFeedrate;
	heaterCombineType = HeaterCombineType::notCombined;
	magic = magicVal;
}

// Load parameters from flash memory
void FlashData::Load()
{
	magic = 0xFFFFFFFF;				// to make sure we know if the read failed
#if SAM4S
	flash_read_user_signature(&(this->magic), (&(this->dummy) - reinterpret_cast<const char*>(&(this->magic)))/sizeof(uint32_t));
#else
	FlashStorage::read(0, &(this->magic), &(this->dummy) - reinterpret_cast<const char*>(&(this->magic)));
#endif
}

// Save parameters to flash memory
void FlashData::Save() const
{
#if SAM4S
	flash_erase_user_signature();
	flash_write_user_signature(&(this->magic), (&(this->dummy) - reinterpret_cast<const char*>(&(this->magic)))/sizeof(uint32_t));
#else
	FlashStorage::write(0, &(this->magic), &(this->dummy) - reinterpret_cast<const char*>(&(this->magic)));
#endif
}

bool FlashData::IsSaveNeeded()
{
	return nvData != savedNvData;
}

void FlashData::SetDisplayDimmerType(DisplayDimmerType newType)
{
	nvData.displayDimmerType = newType;
}

void FlashData::SetVolume(uint8_t newVolume)
{
	nvData.touchVolume = newVolume;
}

void FlashData::SetInfoTimeout(uint8_t newInfoTimeout)
{
	nvData.infoTimeout = newInfoTimeout;
}

void FlashData::SetScreensaverTimeout(uint32_t screensaverTimeout)
{
	nvData.screensaverTimeout = screensaverTimeout;
}

bool FlashData::SetColourScheme(uint8_t newColours)
{
	const bool ret = (newColours != nvData.colourScheme);
	nvData.colourScheme = newColours;
	return ret;
}

// Set the language, returning true if it has changed
bool FlashData::SetLanguage(uint8_t newLanguage)
{
	const bool ret = (newLanguage != nvData.language);
	nvData.language = newLanguage;
	return ret;
}

uint32_t FlashData::GetBaudRate()
{
	return nvData.baudRate;
}

uint32_t FlashData::GetVolume()
{
	return nvData.touchVolume;
}

int FlashData::GetBrightness()
{
	return (int)nvData.brightness;
}

uint32_t FlashData::GetScreensaverTimeout()
{
	return nvData.screensaverTimeout;
}

uint8_t FlashData::GetBabystepAmountIndex()
{
	return nvData.babystepAmountIndex;
}

void FlashData::SetBabystepAmountIndex(uint8_t babystepAmountIndex)
{
	nvData.babystepAmountIndex = babystepAmountIndex;
}

uint16_t FlashData::GetFeedrate()
{
	return nvData.feedrate;
}

void FlashData::SetFeedrate(uint16_t feedrate)
{
	nvData.feedrate = feedrate;
}

HeaterCombineType FlashData::GetHeaterCombineType()
{
	return nvData.heaterCombineType;
}

void FlashData::SetHeaterCombineType(HeaterCombineType combine)
{
	nvData.heaterCombineType = combine;
}

DisplayDimmerType FlashData::GetDisplayDimmerType()
{
	return nvData.displayDimmerType;
}
