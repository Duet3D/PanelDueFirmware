// TFT panel controller to run on SAM3S2B/SAM3S4B/SAM4S4B
// Coding rules:
//
// 1. Must compile with no g++ warnings, when all warning are enabled.
// 2. Dynamic memory allocation using 'new' is permitted during the initialization phase only. No use of 'new' anywhere else,
//    and no use of 'delete', 'malloc' or 'free' anywhere.
// 3. No pure virtual functions. This is because in release builds, having pure virtual functions causes huge amounts of the C++ library to be linked in
//    (possibly because it wants to print a message if a pure virtual function is called).

// Include definitions for verification using Escher C/C++ verifier
#include "ecv.h"

// We have to temporarily allow 'array' and 'result' to be used as ordinary identifiers when including the ASF
#undef array
#undef result
#include "asf.h"

// Reinstate the eCv definitions of 'array' and 'result'
#define array _ecv_array
#define result _ecv_result

#include <cstring>
#include <cctype>

#include "Hardware/Mem.hpp"
#include "Display.hpp"
#include "Hardware/UTFT.hpp"
#include "Hardware/UTouch.hpp"
#include "Hardware/SerialIo.hpp"
#include "Hardware/Buzzer.hpp"
#include "Hardware/SysTick.hpp"
#include "Hardware/Reset.hpp"
#include "Library/Misc.hpp"
#include "Library/Vector.hpp"

#if SAM4S
#include "flash_efc.h"
#else
#include "Hardware/FlashStorage.hpp"
#endif

#include "PanelDue.hpp"
#include "Configuration.hpp"
#include "UserInterfaceConstants.hpp"
#include "FileManager.hpp"
#include "RequestTimer.hpp"
#include "MessageLog.hpp"
#include "Events.hpp"
#include "PrinterStatus.hpp"
#include "UserInterface.hpp"

#ifdef OEM
# if DISPLAY_X == 800
#  include "OemSplashScreen_800_480.hpp"
# else
#  include "OemSplashScreen_480_272.hpp"
# endif
#endif

extern uint16_t _esplash[];							// defined in linker script

#define DEBUG	(0)

// Controlling constants
const uint32_t printerPollInterval = 1000;			// poll interval in milliseconds
const uint32_t printerResponseInterval = 700;		// shortest time after a response that we send another poll (gives printer time to catch up)
const uint32_t printerPollTimeout = 8000;			// poll timeout in milliseconds
const uint32_t FileInfoRequestTimeout = 8000;		// file info request timeout in milliseconds
const uint32_t MachineConfigRequestTimeout = 8000;	// machine configuration timeout in milliseconds
const uint32_t touchBeepLength = 20;				// beep length in ms
const uint32_t touchBeepFrequency = 4500;			// beep frequency in Hz. Resonant frequency of the piezo sounder is 4.5kHz.
const uint32_t errorBeepLength = 100;
const uint32_t errorBeepFrequency = 2250;
const uint32_t longTouchDelay = 250;				// how long we ignore new touches for after pressing Set
const uint32_t shortTouchDelay = 100;				// how long we ignore new touches while pressing up/down, to get a reasonable repeat rate

struct HostFirmwareType
{
	const char* array const name;
	const FirmwareFeatures features;
};

const HostFirmwareType firmwareTypes[] =
{
	{ "RepRapFirmware", quoteFilenames },
	{ "Smoothie", noGcodesFolder | noStandbyTemps | noG10Temps | noDriveNumber | noM20M36 },
	{ "Repetier", noGcodesFolder | noStandbyTemps | noG10Temps }
};

// Variables

#if SAM4S

// Version 3.0
UTFT lcd(DISPLAY_CONTROLLER, 15, 14, 0, 39);
UTouch touch(8, 7, 6, 4, 1);

#elif SAM3S

// Version 1.0, 1.1, 2.0
UTFT lcd(DISPLAY_CONTROLLER, 16, 17, 18, 19);
UTouch touch(23, 24, 22, 21, 20);

#endif

MainWindow mgr;

static uint32_t lastTouchTime;
static uint32_t ignoreTouchTime;
static uint32_t lastPollTime;
static uint32_t lastResponseTime = 0;
static uint32_t lastActionTime = 0;							// the last time anything significant happened
static FirmwareFeatures firmwareFeatures = 0;
static bool isDimmed = false;								// true if we have dimmed the display
static bool isDelta = false;
static bool axisHomed[MaxAxes] = {false, false, false};
static bool allAxesHomed = false;
static size_t numAxes = MIN_AXES;
static int32_t beepFrequency = 0, beepLength = 0;
static uint32_t messageSeq = 0;
static uint32_t newMessageSeq = 0;

const ColourScheme *colours = &colourSchemes[0];

Alert currentAlert;
uint32_t lastAlertSeq = 0;

struct FlashData
{
	// The magic value should be changed whenever the layout of the NVRAM changes
	// We now use a different magic value for each display size, to force the "touch the spot" screen to be displayed when you change the display size
	static const uint32_t magicVal = 0x3AB629F0 + DISPLAY_TYPE;
	static const uint32_t muggleVal = 0xFFFFFFFF;

	uint32_t magic;
	uint32_t baudRate;
	uint16_t xmin;
	uint16_t xmax;
	uint16_t ymin;
	uint16_t ymax;
	DisplayOrientation lcdOrientation;
	DisplayOrientation touchOrientation;
	uint32_t touchVolume;
	uint32_t language;
	uint32_t colourScheme;
	uint32_t brightness;
	uint8_t displayDimmerType;
	uint8_t padding[3];
	char dummy;								// must be at a multiple of 4 bytes from the start because flash is read/written in whole dwords
	
	FlashData() : magic(muggleVal) { }
	bool operator==(const FlashData& other);
	bool operator!=(const FlashData& other) { return !operator==(other); }
	bool IsValid() const; 
	void SetInvalid() { magic = muggleVal; }
	void SetDefaults();
	void Load();
	void Save() const;
};

#if SAM4S
// FlashData must fit in user signature flash area
static_assert(sizeof(FlashData) <= 512, "Flash data too large");
#else
// FlashData must fit in the area we have reserved
static_assert(sizeof(FlashData) <= FLASH_DATA_LENGTH, "Flash data too large");
#endif

FlashData nvData, savedNvData;

static PrinterStatus status = PrinterStatus::connecting;

enum ReceivedDataEvent
{
	rcvUnknown = 0,
	rcvActive,
	rcvAxes,
	rcvDir,
	rcvErr,
	rcvEfactor,
	rcvFilament,
	rcvFiles,
	rcvHeaters,
	rcvHomed,
	rcvHstat,
	rcvPos,
	rcvStandby,
	rcvBeepFreq,
	rcvBeepLength,
	rcvFanPercent,
	rcvFilename,
	rcvFirmwareName,
	rcvFraction,
	rcvGeneratedBy,
	rcvGeometry,
	rcvHeight,
	rcvLastModified,
	rcvLayerHeight,
	rcvMessage,
	rcvMboxMode,
	rcvMboxMsg,
	rcvMboxControls,
	rcvMboxTimeout,
	rcvMboxTitle,
	rcvMboxSeq,
	rcvMyName,
	rcvProbe,
	rcvResponse,
	rcvSeq,
	rcvSfactor,
	rcvSize,
	rcvStatus,
	rcvTimesLeft,
	rcvVolumes,
	rcvNumTools,
	rcvBabystep
};

struct ReceiveDataTableEntry
{
	ReceivedDataEvent rde;
	const char* varName;
};

RequestTimer machineConfigTimer(MachineConfigRequestTimeout, "M408 S1");

bool FlashData::IsValid() const
{
	return magic == magicVal
		&& touchVolume <= Buzzer::MaxVolume
		&& brightness >= Buzzer::MinBrightness
		&& brightness <= Buzzer::MaxBrightness
		&& language < UI::GetNumLanguages()
		&& colourScheme < NumColourSchemes
		&& displayDimmerType < (uint8_t)DisplayDimmerType::NumTypes;
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
		&& displayDimmerType == other.displayDimmerType;
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
	brightness = Buzzer::DefaultBrightness;
	language = 0;
	colourScheme = 0;
	displayDimmerType = (uint8_t)DisplayDimmerType::always;
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

// Return the host firmware features
FirmwareFeatures GetFirmwareFeatures()
{
	return firmwareFeatures;
}

// Strip the drive letter prefix from a file path if the host firmware doesn't support it
const char* array CondStripDrive(const char* array arg)
{
	return ((firmwareFeatures & noDriveNumber) != 0 && isdigit(arg[0]) && arg[1] == ':')
			? arg + 2
			: arg;
}

#if DEBUG
# define STRINGIFY(x)	#x
# define TOSTRING(x)	STRINGIFY(x)
# define ShowLine		debugField->SetValue(TOSTRING(__LINE__)); debugField->Refresh(true, 0, 0)
#else
# define ShowLine		(void)0
#endif

struct FileList
{
	int listNumber;
	size_t scrollOffset;
	String<100> path;
};

void Delay(uint32_t milliSeconds)
{
	const uint32_t now = SystemTick::GetTickCount();
	while (SystemTick::GetTickCount() - now < milliSeconds) { }
}

bool PrintInProgress()
{
	return status == PrinterStatus::printing || status == PrinterStatus::paused || status == PrinterStatus::pausing || status == PrinterStatus::resuming || status == PrinterStatus::simulating;
}

// Search an ordered table for a matching string
ReceivedDataEvent bsearch(const ReceiveDataTableEntry array table[], size_t numElems, const char* key)
{
	size_t low = 0u, high = numElems;
	while (high > low)
	{
		const size_t mid = (high - low)/2 + low;
		const int t = strcasecmp(key, table[mid].varName);
		if (t == 0)
		{
			return table[mid].rde;
		}
		if (t > 0)
		{
			low = mid + 1u;
		}
		else
		{
			high = mid;
		}
	}
	return (low < numElems && strcasecmp(key, table[low].varName) == 0) ? table[low].rde : rcvUnknown;
}

// Return true if sending a command or file list request to the printer now is a good idea.
// We don't want to send these when the printer is busy with a previous command, because they will block normal status requests.
bool OkToSend()
{
	return status == PrinterStatus::idle || status == PrinterStatus::printing || status == PrinterStatus::paused;
}

// Return the printer status
PrinterStatus GetStatus()
{
	return status;
}

void InitLcd(DisplayOrientation dor, uint32_t language, uint32_t colourScheme)
{
	lcd.InitLCD(dor, IS_24BIT, IS_ER);								// set up the LCD
	colours = &colourSchemes[colourScheme];
	UI::CreateFields(language, *colours);							// create all the fields
	lcd.fillScr(black);												// make sure the memory is clear
	Delay(100);														// give the LCD time to update
	RestoreBrightness();											// turn the display on
}

// Ignore touches for a long time
void DelayTouchLong()
{
	lastTouchTime = SystemTick::GetTickCount();
	ignoreTouchTime = longTouchDelay;
}

// Ignore touches for a short time instead of the long time we already asked for
void ShortenTouchDelay()
{
	ignoreTouchTime = shortTouchDelay;
}

void TouchBeep()
{
	Buzzer::Beep(touchBeepFrequency, touchBeepLength, nvData.touchVolume);	
}

void ErrorBeep()
{
	while (Buzzer::Noisy()) { }
	Buzzer::Beep(errorBeepFrequency, errorBeepLength, nvData.touchVolume);
}

// Draw a spot and wait until the user touches it, returning the touch coordinates in tx and ty.
// The alternative X and Y locations are so that the caller can allow for the touch panel being possibly inverted.
void DoTouchCalib(PixelNumber x, PixelNumber y, PixelNumber altX, PixelNumber altY, bool wantY, uint16_t& rawRslt)
{
	const PixelNumber touchCircleRadius = DisplayY/32;
	const PixelNumber touchCalibMaxError = DisplayY/6;
	
	lcd.setColor(colours->labelTextColour);
	lcd.fillCircle(x, y, touchCircleRadius);
	
	for (;;)
	{
		uint16_t tx, ty, rawX, rawY;
		if (touch.read(tx, ty, &rawX, &rawY))
		{
			if (   (abs((int)tx - (int)x) <= touchCalibMaxError || abs((int)tx - (int)altX) <= touchCalibMaxError)
				&& (abs((int)ty - (int)y) <= touchCalibMaxError || abs((int)ty - (int)altY) <= touchCalibMaxError)
			   ) 
			{
				TouchBeep();
				rawRslt = (wantY) ? rawY : rawX;
				break;
			}
		}
	}
	
	lcd.setColor(colours->defaultBackColour);
	lcd.fillCircle(x, y, touchCircleRadius);
}

void CalibrateTouch()
{
	DisplayField *oldRoot = mgr.GetRoot();
	mgr.SetRoot(touchCalibInstruction);
	mgr.Refresh(true);

	touch.init(DisplayX, DisplayY, DefaultTouchOrientAdjust);				// initialize the driver and clear any existing calibration
	
	// Draw spots on the edges of the screen, one at a time, and ask the user to touch them.
	// For the first two, we allow for the touch panel being the wrong way round.
	DoTouchCalib(DisplayX/2, touchCalibMargin, DisplayX/2, DisplayY - 1 - touchCalibMargin, true, nvData.ymin);
	if (nvData.ymin >= 4096/2)
	{
		touch.adjustOrientation(ReverseY);
		nvData.ymin = 4095 - nvData.ymin;
	}
	DoTouchCalib(DisplayX - touchCalibMargin - 1, DisplayY/2, touchCalibMargin, DisplayY/2, false, nvData.xmax);
	if (nvData.xmax < 4096/2)
	{
		touch.adjustOrientation(ReverseX);
		nvData.xmax = 4095 - nvData.xmax;
	}
	DoTouchCalib(DisplayX/2, DisplayY - 1 - touchCalibMargin, DisplayX/2, DisplayY - 1 - touchCalibMargin, true, nvData.ymax);
	DoTouchCalib(touchCalibMargin, DisplayY/2, touchCalibMargin, DisplayY/2, false, nvData.xmin);
	
	nvData.touchOrientation = touch.getOrientation();
	touch.calibrate(nvData.xmin, nvData.xmax, nvData.ymin, nvData.ymax, touchCalibMargin);
	
	mgr.SetRoot(oldRoot);
	mgr.Refresh(true);
}

bool IsSaveAndRestartNeeded()
{
	return nvData.language != savedNvData.language || nvData.colourScheme != savedNvData.colourScheme;
}

bool IsSaveNeeded()
{
	return nvData != savedNvData;
}

void MirrorDisplay()
{
	nvData.lcdOrientation = static_cast<DisplayOrientation>(nvData.lcdOrientation ^ (ReverseX | InvertBitmap));
	lcd.InitLCD(nvData.lcdOrientation, IS_24BIT, IS_ER);
}

void InvertDisplay()
{
	nvData.lcdOrientation = static_cast<DisplayOrientation>(nvData.lcdOrientation ^ (ReverseX | ReverseY | InvertText | InvertBitmap));
	lcd.InitLCD(nvData.lcdOrientation, IS_24BIT, IS_ER);
}

void SetBaudRate(uint32_t rate)
{
	nvData.baudRate = rate;
	SerialIo::Init(rate);
}

extern void SetBrightness(int percent)
{
	nvData.brightness = constrain<int>(percent, Buzzer::MinBrightness, Buzzer::MaxBrightness);
	RestoreBrightness();
}

extern void RestoreBrightness()
{
	Buzzer::SetBacklight(nvData.brightness);
	lastActionTime = SystemTick::GetTickCount();
	isDimmed = false;
}

extern void DimBrightness()
{
	if (   (GetDisplayDimmerType() == DisplayDimmerType::always)
		|| (GetDisplayDimmerType() == DisplayDimmerType::onIdle && status == PrinterStatus::idle)
	   )
	{
		Buzzer::SetBacklight(nvData.brightness/8);
		isDimmed = true;
	}
}

DisplayDimmerType GetDisplayDimmerType()
{
	return (DisplayDimmerType)nvData.displayDimmerType;
}

void SetDisplayDimmerType(DisplayDimmerType newType)
{
	nvData.displayDimmerType = (uint8_t)newType;
}

void SetVolume(uint32_t newVolume)
{
	nvData.touchVolume = newVolume;
}

void SetColourScheme(uint32_t newColours)
{
	nvData.colourScheme = newColours;
}

void SetLanguage(uint32_t newLanguage)
{
	nvData.language = newLanguage;
}

uint32_t GetBaudRate()
{
	return nvData.baudRate;
}

uint32_t GetVolume()
{
	return nvData.touchVolume;
}

int GetBrightness()
{
	return (int)nvData.brightness;
}

// Factory reset
void FactoryReset()
{
	while (Buzzer::Noisy()) { }
	nvData.SetInvalid();
	nvData.Save();
	savedNvData = nvData;
	Buzzer::Beep(touchBeepFrequency, 400, Buzzer::MaxVolume);		// long beep to acknowledge it
	while (Buzzer::Noisy()) { }
	Restart();														// reset the processor
}

// Save settings
void SaveSettings()
{
	while (Buzzer::Noisy()) { }
	nvData.Save();
	// To make sure it worked, load the settings again
	savedNvData.Load();
	UI::CheckSettingsAreSaved();
}

// This is called when the status changes
void SetStatus(char c)
{
	PrinterStatus newStatus;
	switch(c)
	{
	case 'A':
		newStatus = PrinterStatus::paused;
		break;
	case 'B':
		newStatus = PrinterStatus::busy;
		break;
	case 'C':
		newStatus = PrinterStatus::configuring;
		break;
	case 'D':
		newStatus = PrinterStatus::pausing;
		break;
	case 'F':
		newStatus = PrinterStatus::flashing;
		break;
	case 'I':
		newStatus = PrinterStatus::idle;
		break;
	case 'P':
		newStatus = PrinterStatus::printing;
		break;
	case 'M':
		newStatus = PrinterStatus::simulating;
		break;
	case 'R':
		newStatus = PrinterStatus::resuming;
		break;
	case 'S':
		newStatus = PrinterStatus::stopped;
		break;
	case 'T':
		newStatus = PrinterStatus::toolChange;
		break;
	default:
		newStatus = status;		// leave the status alone if we don't recognise it
		break;
	}
	
	if (newStatus != status)
	{
		RestoreBrightness();
		UI::ChangeStatus(status, newStatus);
		
		if (status == PrinterStatus::configuring || (status == PrinterStatus::connecting && newStatus != PrinterStatus::configuring))
		{
			MessageLog::AppendMessage("Connected");
		}
	
		status = newStatus;
		UI::UpdatePrintingFields();
	}
}

// Set the status back to "Connecting"
void Reconnect()
{
	UI::ChangeStatus(status, PrinterStatus::connecting);
	status = PrinterStatus::connecting;
	UI::UpdatePrintingFields();
}

// Try to get an integer value from a string. If it is actually a floating point value, round it.
bool GetInteger(const char s[], int32_t &rslt)
{
	if (s[0] == 0) return false;			// empty string

	char* endptr;
	rslt = (int) strtol(s, &endptr, 10);
	if (*endptr == 0) return true;			// we parsed an integer

	if (strlen(s) > 10) return false;		// avoid strtod buggy behaviour on long input strings

	const float d = strtof(s, &endptr);		// try parsing a floating point number
	if (*endptr == 0)
	{
		rslt = (int)((d < 0.0) ? d - 0.5 : d + 0.5);
		return true;
	}
	return false;
}

// Try to get an unsigned integer value from a string
bool GetUnsignedInteger(const char s[], uint32_t &rslt)
{
	if (s[0] == 0) return false;			// empty string
	char* endptr;
	rslt = (int) strtoul(s, &endptr, 10);
	return *endptr == 0;
}

// Try to get a floating point value from a string. if it is actually a floating point value, round it.
bool GetFloat(const char s[], float &rslt)
{
	if (s[0] == 0) return false;			// empty string

	// GNU strtod is buggy, it's very slow for some long inputs, and some versions have a buffer overflow bug.
	// We presume strtof may be buggy too. Tame it by rejecting any strings that much longer than we expect to receive.
	if (strlen(s) > 10) return false;

	char* endptr;
	rslt = strtof(s, &endptr);
	return *endptr == 0;					// we parsed a float
}

// This table must be kept in case-insensitive alphabetical order of the search string.
// A '^' character indicates the position of an array index, and a ':' character indicates the start of a sub-field name
const ReceiveDataTableEntry fieldTable[] =
{
	{ rcvActive,		"active^" },
	{ rcvAxes,			"axes" },
	{ rcvBabystep,		"babystep" },
	{ rcvBeepFreq,		"beep_freq" },
	{ rcvBeepLength,	"beep_length" },
	{ rcvDir,			"dir" },
	{ rcvEfactor,		"efactor^" },
	{ rcvErr,			"err" },
	{ rcvFanPercent,	"fanPercent^" },
	{ rcvFilament,		"filament^" },
	{ rcvFilename,		"fileName" },
	{ rcvFiles,			"files^" },
	{ rcvFirmwareName,	"firmwareName" },
	{ rcvFraction,		"fraction_printed" },
	{ rcvGeneratedBy,	"generatedBy" },
	{ rcvGeometry,		"geometry" },
	{ rcvHeaters,		"heaters^" },
	{ rcvHeight,		"height" },
	{ rcvHomed,			"homed^" },
	{ rcvHstat,			"hstat^" },
	{ rcvLastModified,	"lastModified" },
	{ rcvLayerHeight,	"layerHeight" },
	{ rcvMessage,		"message" },
	{ rcvMboxControls,	"msgBox.controls" },
	{ rcvMboxMode,		"msgBox.mode" },
	{ rcvMboxMsg,		"msgBox.msg" },
	{ rcvMboxSeq,		"msgBox.seq" },
	{ rcvMboxTimeout,	"msgBox.timeout" },
	{ rcvMboxTitle,		"msgBox.title" },
	{ rcvMyName,		"myName" },
	{ rcvNumTools,		"numTools" },
	{ rcvPos,			"pos^" },
	{ rcvProbe,			"probe" },
	{ rcvResponse,		"resp" },
	{ rcvSeq,			"seq" },
	{ rcvSfactor,		"sfactor" },
	{ rcvSize,			"size" },
	{ rcvStandby,		"standby^" },
	{ rcvStatus,		"status" },
	{ rcvTimesLeft,		"timesLeft^" },
	{ rcvVolumes,		"volumes" }
};

void StartReceivedMessage()
{
	ShowLine;
	newMessageSeq = messageSeq;
	MessageLog::BeginNewMessage();
	FileManager::BeginNewMessage();
	currentAlert.flags = 0;
	ShowLine;
}

void EndReceivedMessage()
{
	ShowLine;
	lastResponseTime = SystemTick::GetTickCount();

	if (newMessageSeq != messageSeq)
	{
		messageSeq = newMessageSeq;
		MessageLog::DisplayNewMessage();
	}	
	FileManager::EndReceivedMessage();
	if ((currentAlert.flags & Alert::GotMode) != 0 && currentAlert.mode < 0)
	{
		UI::ClearAlert();
	}
	else if (currentAlert.flags == Alert::GotAll && currentAlert.seq != lastAlertSeq)
	{
		UI::ProcessAlert(currentAlert);
		lastAlertSeq = currentAlert.seq;
	}
	ShowLine;
}

// Public functions called by the SerialIo module
void ProcessReceivedValue(const char id[], const char data[], const size_t indices[])
{
	ShowLine;
	switch(bsearch(fieldTable, sizeof(fieldTable)/sizeof(fieldTable[0]), id))
	{
	case rcvActive:
		ShowLine;
		{
			int32_t ival;
			if (GetInteger(data, ival))
			{
				UI::UpdateActiveTemperature(indices[0], ival);
			}
		}
		break;

	case rcvStandby:
		ShowLine;
		{
			int32_t ival;
			if (GetInteger(data, ival))
			{
				UI::UpdateStandbyTemperature(indices[0], ival);
			}
		}
		break;

	case rcvHeaters:
		ShowLine;
		{
			float fval;
			if (GetFloat(data, fval))
			{
				ShowLine;
				UI::UpdateCurrentTemperature(indices[0], fval);
			}
		}
		break;

	case rcvHstat:
		ShowLine;
		{
			int32_t ival;
			if (GetInteger(data, ival))
			{
				UI::UpdateHeaterStatus(indices[0], ival);
			}
		}
		break;

	case rcvPos:
		ShowLine;
		{
			float fval;
			if (GetFloat(data, fval))
			{
				UI::UpdateAxisPosition(indices[0], fval);
			}
		}
		break;

	case rcvEfactor:
		ShowLine;
		{
			int32_t ival;
			if (GetInteger(data, ival))
			{
				UI::UpdateExtrusionFactor(indices[0], ival);
			}
		}
		break;

	case rcvFiles:
		ShowLine;
		if (indices[0] == 0)
		{
			FileManager::BeginReceivingFiles();
		}
		FileManager::ReceiveFile(data);
		break;

	case rcvFilament:
		ShowLine;
		{
			static float totalFilament = 0.0;
			if (indices[0] == 0)
			{
				totalFilament = 0.0;
			}
			float f;
			if (GetFloat(data, f))
			{
				totalFilament += f;
				UI::UpdateFileFilament((int)totalFilament);
			}
		}
		break;

	case rcvHomed:
		ShowLine;
		{
			int32_t ival;
			if (indices[0] < MaxAxes && GetInteger(data, ival) && ival >= 0 && ival < 2)
			{
				bool isHomed = (ival == 1);
				if (isHomed != axisHomed[indices[0]])
				{
					axisHomed[indices[0]] = isHomed;
					UI::UpdateHomedStatus(indices[0], isHomed);
					bool allHomed = true;
					for (size_t i = 0; i < numAxes; ++i)
					{
						if (!axisHomed[i])
						{
							allHomed = false;
							break;
						}
					}
					if (allHomed != allAxesHomed)
					{
						allAxesHomed = allHomed;
						UI::UpdateHomedStatus(-1, allHomed);
					}
				}
			}
		}
		break;

	case rcvTimesLeft:
		ShowLine;
		{
			int32_t i;
			bool b = GetInteger(data, i);
			if (b && i >= 0 && i < 10 * 24 * 60 * 60 && PrintInProgress())
			{
				UI::UpdateTimesLeft(indices[0], i);
			}
		}
		break;

	case rcvFanPercent:
		ShowLine;
		if (indices[0] == 0)			// currently we only handle one fan
		{
			float f;
			bool b = GetFloat(data, f);
			if (b && f >= 0.0 && f <= 100.0)
			{
				UI::UpdateFanPercent((int)(f + 0.5));
			}
		}
		break;

	case rcvSfactor:
		{
			int32_t ival;
			if (GetInteger(data, ival))
			{
				UI::UpdateSpeedPercent(ival);
			}
		}
		break;

	case rcvProbe:
		UI::UpdateZProbe(data);
		break;

	case rcvMyName:
		if (status != PrinterStatus::configuring && status != PrinterStatus::connecting)
		{
			UI::UpdateMachineName(data);
		}
		break;

	case rcvFilename:
		UI::PrintingFilenameChanged(data);
		break;

	case rcvSize:
		{
			int32_t sz;
			if (GetInteger(data, sz))
			{
				UI::UpdateFileSize(sz);
			}
		}
		break;

	case rcvHeight:
		{
			float f;
			if (GetFloat(data, f))
			{
				UI::UpdateFileObjectHeight(f);
			}
		}
		break;

	case rcvLastModified:
		UI::UpdateFileLastModifiedText(data);
		break;

	case rcvLayerHeight:
		{
			float f;
			if (GetFloat(data, f))
			{
				UI::UpdateFileLayerHeight(f);
			}
		}
		break;

	case rcvGeneratedBy:
		UI::UpdateFileGeneratedByText(data);
		break;

	case rcvFraction:
		{
			float f;
			if (GetFloat(data, f))
			{
				if (f >= 0.0 && f <= 1.0)
				{
					UI::SetPrintProgressPercent((unsigned int)(100.0 * f) + 0.5);
				}
			}
		}
		break;

	case rcvStatus:
		SetStatus(data[0]);
		break;

	case rcvBeepFreq:
		GetInteger(data, beepFrequency);
		break;

	case rcvBeepLength:
		GetInteger(data, beepLength);
		break;

	case rcvGeometry:
		if (status != PrinterStatus::configuring && status != PrinterStatus::connecting)
		{
			isDelta = (strcasecmp(data, "delta") == 0);
			UI::UpdateGeometry(numAxes, isDelta);
		}
		break;

	case rcvAxes:
		{
			uint32_t n = MIN_AXES;
			GetUnsignedInteger(data, n);
			numAxes = constrain<unsigned int>(n, MIN_AXES, MaxAxes);
			UI::UpdateGeometry(numAxes, isDelta);
		}
		break;

	case rcvSeq:
		GetUnsignedInteger(data, newMessageSeq);
		break;

	case rcvResponse:
		MessageLog::SaveMessage(data);
		break;

	case rcvDir:
		FileManager::ReceiveDirectoryName(data);
		break;

	case rcvMessage:
		if (data[0] == 0)
		{
			UI::ClearAlert();
		}
		else
		{
			UI::ProcessSimpleAlert(data);
		}
		break;

	case rcvMboxMode:
		if (GetInteger(data, currentAlert.mode))
		{
			currentAlert.flags |= Alert::GotMode;
		}
		break;

	case rcvMboxMsg:
		currentAlert.text.copy(data);
		currentAlert.flags |= Alert::GotText;
		break;

	case rcvMboxControls:
		if (GetUnsignedInteger(data, currentAlert.controls))
		{
			currentAlert.flags |= Alert::GotControls;
		}
		break;

	case rcvMboxTimeout:
		if (GetFloat(data, currentAlert.timeout))
		{
			currentAlert.flags |= Alert::GotTimeout;
		}
		break;

	case rcvMboxTitle:
		currentAlert.title.copy(data);
		currentAlert.flags |= Alert::GotTitle;
		break;

	case rcvMboxSeq:
		if (GetUnsignedInteger(data, currentAlert.seq))
		{
			currentAlert.flags |= Alert::GotSeq;
		}
		break;

	case rcvErr:
		{
			int32_t i;
			if (GetInteger(data, i))
			{
				FileManager::ReceiveErrorCode(i);
			}
		}
		break;

	case rcvVolumes:
		{
			uint32_t i;
			if (GetUnsignedInteger(data, i))
			{
				FileManager::SetNumVolumes(i);
			}
		}
		break;

	case rcvNumTools:
		{
			uint32_t i;
			if (GetUnsignedInteger(data, i))
			{
				UI::SetNumTools(i);
			}
		}
		break;

	case rcvFirmwareName:
		for (size_t i = 0; i < ARRAY_SIZE(firmwareTypes); ++i)
		{
			if (stringStartsWith(data, firmwareTypes[i].name))
			{
				const FirmwareFeatures newFeatures = firmwareTypes[i].features;
				if (newFeatures != firmwareFeatures)
				{
					firmwareFeatures = newFeatures;
					UI::FirmwareFeaturesChanged(firmwareFeatures);
					FileManager::FirmwareFeaturesChanged();
				}
				break;
			}
		}
		break;

	case rcvBabystep:
		{
			float f;
			if (GetFloat(data, f))
			{
				UI::SetBabystepOffset(f);
			}
		}
		break;

	default:
		break;
	}
	ShowLine;
}

// Public function called when the serial I/O module finishes receiving an array of values
void ProcessArrayEnd(const char id[], const size_t indices[])
{
	if (indices[0] == 0 && strcmp(id, "files^") == 0)
	{
		FileManager::BeginReceivingFiles();				// received an empty file list - need to tell the file manager about it
	}
	if (strcmp(id, "heaters^") == 0)
	{
		UI::SetNumHeaters(indices[0]);					// tell the user interface how many heaters there are
	}
}

// Update those fields that display debug information
void UpdateDebugInfo()
{
	freeMem->SetValue(getFreeMemory());
}

#if 0
void SelfTest()
{
	// Measure the 3.3V supply against the internal reference
	
	// Do internal and external loopback tests on the serial port

	// Initialize fields with the widest expected values so that we can make sure they fit
	currentTemps[0]->SetValue(129.0);
	currentTemps[1]->SetValue(299.0);
	currentTemps[2]->SetValue(299.0);
	activeTemps[0]->SetValue(120);
	activeTemps[1]->SetValue(280);
	activeTemps[2]->SetValue(280);
	standbyTemps[1]->SetValue(280);
	standbyTemps[2]->SetValue(280);
	axisPos[0]->SetValue(220.9);
	axisPos[1]->SetValue(220.9);
	axisPos[2]->SetValue(199.99);
	zProbe->SetValue("1023 (1023)");
	spd->SetValue(169);
	extrusionFactors[0]->SetValue(169);
	extrusionFactors[1]->SetValue(169);
}
#endif

/**
 * \brief Application entry point.
 *
 * \return Unused (ANSI-C compatibility).
 */
int main(void)
{
    SystemInit();						// set up the clock etc.	
	
	matrix_set_system_io(CCFG_SYSIO_SYSIO4 | CCFG_SYSIO_SYSIO5 | CCFG_SYSIO_SYSIO6 | CCFG_SYSIO_SYSIO7);	// enable PB4-PB7 pins
	pmc_enable_periph_clk(ID_PIOA);		// enable the PIO clock
	pmc_enable_periph_clk(ID_PIOB);		// enable the PIO clock
	pmc_enable_periph_clk(ID_PWM);		// enable the PWM clock
#if SAM4S
	pmc_enable_periph_clk(ID_UART0);	// enable UART1 clock
#else
	pmc_enable_periph_clk(ID_UART1);	// enable UART1 clock
#endif

	Buzzer::Init();						// init the buzzer, must also call this before the backlight can be used

	wdt_init (WDT, WDT_MR_WDRSTEN, 1000, 1000);
	SysTick_Config(SystemCoreClock / 1000);
	lastTouchTime = SystemTick::GetTickCount();

	firmwareFeatures = firmwareTypes[0].features;		// assume RepRapFirmware until we hear otherwise

	// Read parameters from flash memory
	nvData.Load();
	if (nvData.IsValid())
	{
		// The touch panel has already been calibrated
		InitLcd(nvData.lcdOrientation, nvData.language, nvData.colourScheme);
		touch.init(DisplayX, DisplayY, nvData.touchOrientation);
		touch.calibrate(nvData.xmin, nvData.xmax, nvData.ymin, nvData.ymax, touchCalibMargin);
		savedNvData = nvData;
	}
	else
	{
		// The touch panel has not been calibrated, and we do not know which way up it is
		nvData.SetDefaults();
		InitLcd(nvData.lcdOrientation, nvData.language, nvData.colourScheme);
		CalibrateTouch();							// this includes the touch driver initialisation
		SaveSettings();
	}
	
	// Set up the baud rate
	SerialIo::Init(nvData.baudRate);
	
	MessageLog::Init();

#ifdef OEM
	// Display the splash screen unless it was a software reset (we use software reset to change the language or colour scheme)
	if (rstc_get_reset_cause(RSTC) != RSTC_SOFTWARE_RESET)
	{
		lcd.fillScr(black);
		lcd.drawCompressedBitmapBottomToTop(0, 0, DISPLAY_X, DISPLAY_Y, splashScreenImage);
		Delay(5000);								// hold it there for 5 seconds
	}
#else
	// Display the splash screen if one has been appended to the file, unless it was a software reset (we use software reset to change the language or colour scheme)
	// The splash screen data comprises the number of X pixels, then the number of Y pixels, then the data
	if (rstc_get_reset_cause(RSTC) != RSTC_SOFTWARE_RESET && _esplash[0] == DISPLAY_X && _esplash[1] == DISPLAY_Y)
	{
		lcd.fillScr(black);
		lcd.drawCompressedBitmapBottomToTop(0, 0, DISPLAY_X, DISPLAY_Y, _esplash  + 2);
		const uint32_t now = SystemTick::GetTickCount();
		do
		{
			uint16_t x, y;
			if (touch.read(x, y))
			{
				break;
			}
		} while (SystemTick::GetTickCount() - now < 5000);		// hold it there for 5 seconds or until touched
	}
#endif

	mgr.Refresh(true);								// draw the screen for the first time
	UI::UpdatePrintingFields();

	lastPollTime = SystemTick::GetTickCount() - printerPollInterval;	// allow a poll immediately
	
	// Hide the Head 2+ parameters until we know we have a second head
	UI::SetNumHeaters(2);
	UI::SetNumTools(1);
	
	debugField->Show(DEBUG != 0);					// show the debug field only if debugging is enabled

	// Display the Control tab. This also refreshes the display.
	UI::ShowDefaultPage();
	lastResponseTime = SystemTick::GetTickCount();	// pretend we just received a response
	
	machineConfigTimer.SetPending();				// we need to fetch the machine name and configuration
	lastActionTime = SystemTick::GetTickCount();

	for (;;)
	{
		ShowLine;

		// 1. Check for input from the serial port and process it.
		// This calls back into functions StartReceivedMessage, ProcessReceivedValue, ProcessArrayLength and EndReceivedMessage.
		SerialIo::CheckInput();
		ShowLine;
		
		// 2. if displaying the message log, update the times
		UI::Spin();
		ShowLine;
		
		// 3. Check for a touch on the touch panel.
		if (SystemTick::GetTickCount() - lastTouchTime >= ignoreTouchTime)
		{
			UI::OnButtonPressTimeout();

			uint16_t x, y;
			if (touch.read(x, y))
			{
#if 0
				touchX->SetValue((int)x);	//debug
				touchY->SetValue((int)y);	//debug
#endif
				if (isDimmed)
				{
					RestoreBrightness();
					DelayTouchLong();			// ignore further touches for a while
				}
				else
				{
					lastActionTime = SystemTick::GetTickCount();
					ButtonPress bp = mgr.FindEvent(x, y);
					if (bp.IsValid())
					{
						DelayTouchLong();		// by default, ignore further touches for a long time
						if (bp.GetEvent() != evAdjustVolume)
						{
							TouchBeep();		// give audible feedback of the touch, unless adjusting the volume
						}
						UI::ProcessTouch(bp);
					}
					else
					{
						bp = mgr.FindEventOutsidePopup(x, y);
						if (bp.IsValid())
						{
							UI::ProcessTouchOutsidePopup(bp);
						}
					}
				}
			}
			else if (!isDimmed && SystemTick::GetTickCount() - lastActionTime >= DimDisplayTimeout && UI::CanDimDisplay())
			{
				DimBrightness();				// it might not actually dim the display, depending on various flags
 			}
		}
		ShowLine;
		
		// 4. Refresh the display
		UpdateDebugInfo();
		mgr.Refresh(false);
		ShowLine;
		
		// 5. Generate a beep if asked to
		if (beepFrequency != 0 && beepLength != 0)
		{
			if (beepFrequency >= 100 && beepFrequency <= 10000 && beepLength > 0)
			{
				if (beepLength > 20000)
				{
					beepLength = 20000;			// limit the beep to 20 seconds
				}
				Buzzer::Beep(beepFrequency, beepLength, Buzzer::MaxVolume);
			}
			beepFrequency = beepLength = 0;
		}
		ShowLine;

		// 6. If it is time, poll the printer status.
		// When the printer is executing a homing move or other file macro, it may stop responding to polling requests.
		// Under these conditions, we slow down the rate of polling to avoid building up a large queue of them.
		const uint32_t now = SystemTick::GetTickCount();
		if (   UI::DoPolling()										// don't poll while we are in the Setup page
		    && now - lastPollTime >= printerPollInterval			// if we haven't polled the printer too recently...
			&& now - lastResponseTime >= printerResponseInterval	// and we haven't had a response too recently
		   )
		{
			if (now - lastPollTime > now - lastResponseTime)		// if we've had a response since the last poll
			{
				// First check for specific info we need to fetch
				bool done = machineConfigTimer.Process();
				if (!done)
				{
					done = FileManager::ProcessTimers();
				}
				
				// Otherwise just send a normal poll command
				if (!done)
				{
					SerialIo::SendString("M408 S0 R");
					SerialIo::SendInt(messageSeq);
					SerialIo::SendChar('\n');
				}
				lastPollTime = SystemTick::GetTickCount();
			}
			else if (now - lastPollTime >= printerPollTimeout)		// if we're giving up on getting a response to the last poll
			{
				SerialIo::SendString("M408 S0\n");
				lastPollTime = SystemTick::GetTickCount();
			}
		}
		ShowLine;
	}
}

void PrintDebugText(const char *x)
{
	fwVersionField->SetValue(x);
}

// Pure virtual function call handler, to avoid pulling in large chunks of the standard library
extern "C" void __cxa_pure_virtual() { while (1); }

// End
