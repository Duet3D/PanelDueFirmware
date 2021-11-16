/*
 * PanelDue.hpp
 *
 * Created: 06/12/2014 14:23:38
 *  Author: David
 */

#ifndef PANELDUE_H_
#define PANELDUE_H_

#include "ecv.h"
#undef array
#undef result
#undef value
#include <Hardware/UTFT.hpp>
#include <UI/Display.hpp>
#include "RequestTimer.hpp"
#include "FirmwareFeatures.hpp"
#include <ObjectModel/PrinterStatus.hpp>
#include <General/String.h>

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

// Functions called from module UserInterface
extern bool IsPrintingStatus(OM::PrinterStatus status);
extern bool PrintInProgress();
extern OM::PrinterStatus GetStatus();
extern void DelayTouchLong();
extern void ShortenTouchDelay();
extern void TouchBeep();
extern void ErrorBeep();
extern void CalibrateTouch();

// Functions called from module UserInterface to manipulate non-volatile settings and associated hardware
extern void FactoryReset();
extern void SaveSettings();
extern bool IsSaveNeeded();
extern void MirrorDisplay();
extern void InvertDisplay();
extern void LandscapeDisplay(const bool withTouch = true);
extern void PortraitDisplay(const bool withTouch = true);
extern void SetBaudRate(uint32_t rate);
extern void SetBrightness(int percent);
extern void RestoreBrightness();
extern void SetVolume(uint8_t newVolume);
extern void SetInfoTimeout(uint8_t newInfoTimeout);
extern void SetScreensaverTimeout(uint32_t screensaverTimeout);
extern bool GetScreensaverActive();
extern bool SetColourScheme(uint8_t newColours);
extern bool SetLanguage(uint8_t newLanguage);
extern uint32_t GetBaudRate();
extern int GetBrightness();
extern uint32_t GetVolume();
extern uint32_t GetScreensaverTimeout();
extern DisplayDimmerType GetDisplayDimmerType();
extern void SetDisplayDimmerType(DisplayDimmerType newType);
extern uint8_t GetBabystepAmountIndex();
extern void SetBabystepAmountIndex(uint8_t babystepAmountIndex);
extern uint16_t GetFeedrate();
extern void SetFeedrate(uint16_t feedrate);
extern HeaterCombineType GetHeaterCombineType();
extern void SetHeaterCombineType(HeaterCombineType combine);
extern FirmwareFeatureMap GetFirmwareFeatures();
extern const char* _ecv_array CondStripDrive(const char* _ecv_array arg);
extern void Delay(uint32_t milliSeconds);

// Global data in PanelDue.cpp that is used elsewhere
extern UTFT lcd;
extern MainWindow mgr;

class ColourScheme;
extern const ColourScheme *colours;

const size_t MIN_AXES = 2;					// the minimum number of axes we support

const size_t alertTextLength = 165;			// maximum characters in the alert text
const size_t alertTitleLength = 50;			// maximum characters in the alert title

struct Alert
{
	int32_t mode;
	uint32_t seq;
	uint32_t controls;
	float timeout;
	String<50> title;
	String<alertTextLength> text;
	Bitmap<uint8_t> flags;

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

#endif /* PANELDUE_H_ */
