// Controller for Ormerod to run on SAM3S2B
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
#include "Hardware/FlashStorage.hpp"
#include "PanelDue.hpp"
#include "Configuration.hpp"
#include "Fields.hpp"
#include "FileManager.hpp"
#include "RequestTimer.hpp"
#include "MessageLog.hpp"

#ifdef OEM
# if DISPLAY_X == 800
#  include "OemSplashScreen_800_480.hpp"
# else
#  include "OemSplashScreen_480_272.hpp"
# endif
#endif

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
const size_t maxUserCommandLength = 40;				// max length of a user gcode command
const size_t numUserCommandBuffers = 6;				// number of command history buffers plus one

// Variables
UTFT lcd(DISPLAY_CONTROLLER, TMode16bit, 16, 17, 18, 19);

UTouch touch(23, 24, 22, 21, 20);
MainWindow mgr;

const char* array null currentFile;					// file whose info is displayed in the file info popup

static uint32_t lastTouchTime;
static uint32_t ignoreTouchTime;
static uint32_t lastPollTime;
static uint32_t lastResponseTime = 0;
static bool gotMachineName = false;
static bool isDelta = false;
static bool gotGeometry = false;
static bool axisHomed[MAX_AXES] = {false, false, false};
static bool allAxesHomed = false;
static size_t numAxes = MIN_AXES;
static int beepFrequency = 0, beepLength = 0;
static unsigned int numHeads = 1;
static unsigned int messageSeq = 0;
static unsigned int newMessageSeq = 0;
static int oldIntValue;
static bool keyboardIsDisplayed = false;
static bool restartNeeded = false;

static int timesLeft[3];
static String<50> timesLeftText;
static String<maxUserCommandLength> userCommandBuffers[numUserCommandBuffers];
static size_t currentUserCommandBuffer = 0, currentHistoryBuffer = 0;

const ColourScheme *colours = &colourSchemes[0];

struct FlashData
{
	static const uint32_t magicVal = 0x3AB629D1;
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
	char dummy;
	
	FlashData() : magic(muggleVal) { }
	bool operator==(const FlashData& other);
	bool operator!=(const FlashData& other) { return !operator==(other); }
	bool IsValid() const; 
	void SetInvalid() { magic = muggleVal; }
	void SetDefaults();
	void Load();
	void Save() const;
};

static_assert(sizeof(FlashData) <= FLASH_DATA_LENGTH, "Flash data too large");

FlashData nvData, savedNvData;

enum class PrinterStatus
{
	connecting = 0,
	idle = 1,
	printing = 2,
	stopped = 3,
	configuring = 4,
	paused = 5,
	busy = 6,
	pausing = 7,
	resuming = 8,
	flashing = 9
};

// Map of the above status codes to text. The space at the end improves the appearance.
const char *statusText[] =
{
	"Connecting",
	"Idle ",
	"Printing ",
	"Halted",
	"Starting up ",
	"Paused ",
	"Busy ",
	"Pausing ",
	"Resuming ",
	"Firmware upload"
};

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
	rcvFraction,
	rcvGeneratedBy,
	rcvGeometry,
	rcvHeight,
	rcvLayerHeight,
	rcvMessage,
	rcvMyName,
	rcvProbe,
	rcvResponse,
	rcvSeq,
	rcvSfactor,
	rcvSize,
	rcvStatus,
	rcvTimesLeft,
	rcvVolumes
};

struct ReceiveDataTableEntry
{
	ReceivedDataEvent rde;
	const char* varName;
};

static Event eventToConfirm = evNull;

int heaterStatus[maxHeaters];

RequestTimer fileInfoTimer(FileInfoRequestTimeout, "M36");
RequestTimer machineConfigTimer(MachineConfigRequestTimeout, "M408 S1");

bool FlashData::IsValid() const
{
	return magic == magicVal && touchVolume <= Buzzer::MaxVolume && brightness <= Buzzer::MaxBrightness && language < numLanguages && colourScheme < NumColourSchemes;
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
		&& brightness == other.brightness;
}

void FlashData::SetDefaults()
{
	baudRate = DEFAULT_BAUD_RATE;
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
	magic = magicVal;
}

// Load parameters from flash memory
void FlashData::Load()
{
	FlashStorage::read(0, &(this->magic), &(this->dummy) - (char*)(&(this->magic)));
}

// Save parameters to flash memory
void FlashData::Save() const
{
	FlashStorage::write(0, &(this->magic), &(this->dummy) - (const char*)(&(this->magic)));
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

bool PrintInProgress()
{
	return status == PrinterStatus::printing || status == PrinterStatus::paused || status == PrinterStatus::pausing || status == PrinterStatus::resuming;
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

void ChangeTab(ButtonBase *newTab)
{
	if (newTab != currentTab)
	{
		if (currentTab != NULL)
		{
			currentTab->Press(false, 0);
		}
		newTab->Press(true, 0);
		currentTab = newTab;
		mgr.ClearAllPopups();
		switch(newTab->GetEvent())
		{
		case evTabControl:
			mgr.SetRoot(controlRoot);
			nameField->SetValue(machineName.c_str());
			break;
		case evTabPrint:
			mgr.SetRoot(printRoot);
			nameField->SetValue(PrintInProgress() ? printingFile.c_str() : machineName.c_str());
			FileManager::RefreshFilesList();
			break;
		case evTabMsg:
			mgr.SetRoot(messageRoot);
			if (keyboardIsDisplayed)
			{
				mgr.SetPopup(keyboardPopup, margin, (DisplayX - keyboardPopupWidth)/2, false);
			}
			break;
		case evTabSetup:
			mgr.SetRoot(setupRoot);
			break;
		default:
			mgr.SetRoot(commonRoot);
			break;
		}

		if (currentButton.GetButton() == newTab)
		{
			currentButton.Clear();									// to prevent it being released
		}
		mgr.Refresh(true);
	}
}

void InitLcd(DisplayOrientation dor, uint32_t language, uint32_t colourScheme)
{
	lcd.InitLCD(dor, is24BitLcd);									// set up the LCD
	colours = &colourSchemes[colourScheme];
	Fields::CreateFields(language, *colours);						// create all the fields
	mgr.Refresh(true);												// redraw everything

	currentTab = NULL;
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
	touchCalibInstruction->SetValue("Touch the spot");				// in case the user didn't need to press the reset button last time
	mgr.SetRoot(touchCalibInstruction);
	mgr.ClearAll();
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
	mgr.ClearAll();
	mgr.Refresh(true);
}

void CheckSettingsAreSaved()
{
	Fields::SettingsAreSaved(nvData == savedNvData, nvData.colourScheme != savedNvData.colourScheme);
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
	CheckSettingsAreSaved();
}

void PopupAreYouSure(Event ev, const char* text, const char* query = "Are you sure?")
{
	eventToConfirm = ev;
	areYouSureTextField->SetValue(text);
	areYouSureQueryField->SetValue(query);
	mgr.SetPopup(areYouSurePopup, (DisplayX - areYouSurePopupWidth)/2, (DisplayY - areYouSurePopupHeight)/2);
}

void PopupRestart()
{
	PopupAreYouSure(evRestart, "Restart required", "Restart now?");
}

void Adjusting(ButtonPress bp)
{
	fieldBeingAdjusted = bp;
	if (bp == currentButton)
	{
		currentButton.Clear();		// to stop it being released
	}
}

void StopAdjusting()
{
	if (fieldBeingAdjusted.IsValid())
	{
		mgr.Press(fieldBeingAdjusted, false);
		fieldBeingAdjusted.Clear();
	}
}

void CurrentButtonReleased()
{
	if (currentButton.IsValid())
	{
		mgr.Press(currentButton, false);
		currentButton.Clear();	
	}
}

// Nasty hack to work around bug in RepRapFirmware 1.09k and earlier
// The M23 and M30 commands don't work if we send the full path, because "0:/gcodes/" gets prepended regardless.
const char * array StripPrefix(const char * array dir)
{
	const size_t len = strlen(dir);
	if (len >= 8 && memcmp(dir, "/gcodes/", 8) == 0)
	{
		dir += 8;
	}
	else if (len >= 10 && memcmp(dir, "0:/gcodes/", 10) == 0)
	{
		dir += 10;
	}
	else if (strcmp(dir, "/gcodes") == 0 || strcmp(dir, "0:/gcodes") == 0)
	{
		dir += len;
	}
	return dir;
}

// Process a touch event
void ProcessTouch(ButtonPress bp)
{
	if (bp.IsValid())
	{
		ButtonBase *f = bp.GetButton();
		currentButton = bp;
		mgr.Press(bp, true);
		Event ev = (Event)(f->GetEvent());
		switch(ev)
		{
		case evTabControl:
		case evTabPrint:
		case evTabMsg:
		case evTabSetup:
			ChangeTab(f);
			break;

		case evAdjustActiveTemp:
		case evAdjustStandbyTemp:
			if (static_cast<IntegerButton*>(f)->GetValue() < 0)
			{
				static_cast<IntegerButton*>(f)->SetValue(0);
			}
			Adjusting(bp);
			mgr.SetPopup(setTempPopup, tempPopupX, popupY);
			break;

		case evAdjustSpeed:
		case evExtrusionFactor:
		case evAdjustFan:
			oldIntValue = static_cast<IntegerButton*>(bp.GetButton())->GetValue();
			Adjusting(bp);
			mgr.SetPopup(setTempPopup, tempPopupX, popupY);
			break;

		case evSetInt:
			if (fieldBeingAdjusted.IsValid())
			{
				int val = static_cast<const IntegerButton*>(fieldBeingAdjusted.GetButton())->GetValue();
				switch(fieldBeingAdjusted.GetEvent())
				{
				case evAdjustActiveTemp:
					{
						int heater = fieldBeingAdjusted.GetIParam();
						if (heater == 0)
						{
							SerialIo::SendString("M140 S");
							SerialIo::SendInt(val);
							SerialIo::SendChar('\n');
						}
						else
						{
							SerialIo::SendString("G10 P");
							SerialIo::SendInt(heater - 1);
							SerialIo::SendString(" S");
							SerialIo::SendInt(val);
							SerialIo::SendChar('\n');
						}
					}
					break;
					
				case evAdjustStandbyTemp:
					{
						int heater = fieldBeingAdjusted.GetIParam();
						if (heater > 0)
						{
							SerialIo::SendString("G10 P");
							SerialIo::SendInt(heater - 1);
							SerialIo::SendString(" R");
							SerialIo::SendInt(val);
							SerialIo::SendChar('\n');
						}
					}
					break;
				
				case evExtrusionFactor:
					{
						int heater = fieldBeingAdjusted.GetIParam();
						SerialIo::SendString("M221 P");
						SerialIo::SendInt(heater);
						SerialIo::SendString(" S");
						SerialIo::SendInt(val);
						SerialIo::SendChar('\n');
					}
					break;
					
				case evAdjustFan:
					SerialIo::SendString("M106 S");
					SerialIo::SendInt((256 * val)/100);
					SerialIo::SendChar('\n');
					break;

				default:
					{
						const char* null cmd = fieldBeingAdjusted.GetSParam();
						if (cmd != NULL)
						{
							SerialIo::SendString(cmd);
							SerialIo::SendInt(val);
							SerialIo::SendChar('\n');
						}
					}
					break;
				}
				mgr.ClearPopup();
				StopAdjusting();
			}
			break;

		case evAdjustInt:
			if (fieldBeingAdjusted.IsValid())
			{
				IntegerButton *ib = static_cast<IntegerButton*>(fieldBeingAdjusted.GetButton());
				int newValue = ib->GetValue() + bp.GetIParam();
				switch(fieldBeingAdjusted.GetEvent())
				{
				case evAdjustActiveTemp:
				case evAdjustStandbyTemp:
					newValue = max<int>(0, min<int>(300, newValue));
					break;

				case evAdjustFan:
					newValue = max<int>(0, min<int>(100, newValue));
					break;

				default:
					break;
				}
				ib->SetValue(newValue);
				ShortenTouchDelay();
			}
			break;

		case evMovePopup:
			mgr.SetPopup(movePopup, movePopupX, movePopupY);
			break;

		case evMoveX:
		case evMoveY:
		case evMoveZ:
		case evMoveU:
		case evMoveV:
		case evMoveW:
			{
				const uint8_t axis = ev - evMoveX;
				const char c = (axis < 3) ? 'X' + axis : ('U' - 3) + axis;
				SerialIo::SendString("G91\nG1 ");
				SerialIo::SendChar(c);
				SerialIo::SendString(bp.GetSParam());
				SerialIo::SendString(" F6000\nG90\n");
			}
			break;

		case evExtrudePopup:
			mgr.SetPopup(extrudePopup, extrudePopupX, extrudePopupY);
			break;

		case evExtrudeAmount:
			mgr.Press(currentExtrudeAmountPress, false);
			mgr.Press(bp, true);
			currentExtrudeAmountPress = bp;
			currentButton.Clear();						// stop it being released by the timer
			break;

		case evExtrudeRate:
			mgr.Press(currentExtrudeRatePress, false);
			mgr.Press(bp, true);
			currentExtrudeRatePress = bp;
			currentButton.Clear();						// stop it being released by the timer
			break;

		case evExtrude:
		case evRetract:
			if (currentExtrudeAmountPress.IsValid() && currentExtrudeRatePress.IsValid())
			{
				SerialIo::SendString("G92 E0\nG1 E");
				if (ev == evRetract)
				{
					SerialIo::SendChar('-');
				}
				SerialIo::SendString(currentExtrudeAmountPress.GetSParam());
				SerialIo::SendString(" F");
				SerialIo::SendString(currentExtrudeRatePress.GetSParam());
				SerialIo::SendChar('\n');
			}
			break;

		case evListFiles:
			FileManager::DisplayFilesList();
			break;

		case evListMacros:
			FileManager::DisplayMacrosList();
			break;

		case evCalTouch:
			CalibrateTouch();
			CheckSettingsAreSaved();
			break;

		case evFactoryReset:
			PopupAreYouSure(ev, "Confirm factory reset");
			break;

		case evRestart:
			PopupAreYouSure(ev, "Confirm restart");
			break;

		case evSaveSettings:
			SaveSettings();
			if (restartNeeded)
			{
				PopupRestart();
			}
			break;

		case evSelectHead:
			{
				int head = bp.GetIParam();
				if (head == 0)
				{
					if (heaterStatus[0] == 2)			// if bed is active
					{
						SerialIo::SendString("M144\n");
					}
					else
					{
						SerialIo::SendString("M140 S");
						SerialIo::SendInt(activeTemps[0]->GetValue());
						SerialIo::SendChar('\n');
					}
				}
				else if (head < (int)maxHeaters)
				{
					if (heaterStatus[head] == 2)		// if head is active
					{
						SerialIo::SendString("T-1\n");
					}
					else
					{
						SerialIo::SendChar('T');
						SerialIo::SendInt(head - 1);
						SerialIo::SendChar('\n');
					}
				}
			}
			break;
	
		case evFile:
			{
				const char * array fileName = bp.GetSParam();
				if (fileName != nullptr)
				{
					if (fileName[0] == '*')
					{
						// It's a directory
						FileManager::RequestFilesSubdir(fileName + 1);
						//??? need to pop up a "wait" box here
					}
					else
					{
						// It's a regular file
						currentFile = fileName;
						SerialIo::SendString("M36 ");			// ask for the file info
						SerialIo::SendFilename(FileManager::GetFilesDir(), currentFile);
						SerialIo::SendChar('\n');
						fpNameField->SetValue(currentFile);
						// Clear out the old field values, they relate to the previous file we looked at until we process the response
						fpSizeField->SetValue(0);						// would be better to make it blank
						fpHeightField->SetValue(0.0);					// would be better to make it blank
						fpLayerHeightField->SetValue(0.0);				// would be better to make it blank
						fpFilamentField->SetValue(0);					// would be better to make it blank
						generatedByText.clear();
						fpGeneratedByField->SetChanged();
						mgr.SetPopup(filePopup, (DisplayX - fileInfoPopupWidth)/2, (DisplayY - fileInfoPopupHeight)/2);
					}
				}
				else
				{
					ErrorBeep();
				}
			}
			break;

		case evFilesUp:
			FileManager::RequestFilesParentDir();
			break;

		case evMacrosUp:
			FileManager::RequestMacrosParentDir();
			break;

		case evMacro:
			{
				const char *fileName = bp.GetSParam();
				if (fileName != nullptr)
				{
					if (fileName[0] == '*')		// if it's a directory
					{
						FileManager::RequestMacrosSubdir(fileName + 1);
						//??? need to pop up a "wait" box here					
					}
					else
					{
						SerialIo::SendString("M98 P");
						SerialIo::SendFilename(FileManager::GetMacrosDir(), fileName);
						SerialIo::SendChar('\n');
					} 
				}
				else
				{
					ErrorBeep();
				}
			}
			break;

		case evPrint:
			mgr.ClearPopup();			// clear the file info popup
			mgr.ClearPopup();			// clear the file list popup
			if (currentFile != nullptr)
			{
				SerialIo::SendString("M32 ");
				SerialIo::SendFilename(StripPrefix(FileManager::GetFilesDir()), currentFile);
				SerialIo::SendChar('\n');
				printingFile.copy(currentFile);
				currentFile = nullptr;							// allow the file list to be updated
				CurrentButtonReleased();
				ChangeTab(tabPrint);
			}
			break;

		case evCancel:
			eventToConfirm = evNull;
			currentFile = nullptr;
			CurrentButtonReleased();
			if (mgr.GetPopup() == keyboardPopup)
			{
				keyboardIsDisplayed = false;
			}
			mgr.ClearPopup();
			break;

		case evDeleteFile:
			CurrentButtonReleased();;
			PopupAreYouSure(ev, "Confirm file delete");
			break;

		case evSendCommand:
		case evPausePrint:
		case evResumePrint:
		case evReset:
			SerialIo::SendString(bp.GetSParam());
			SerialIo::SendChar('\n');
			break;

		case evScrollFiles:
			FileManager::Scroll(bp.GetIParam());
			ShortenTouchDelay();				
			break;

		case evChangeCard:
			FileManager::ChangeCard();
			break;

		case evKeyboard:
			mgr.SetPopup(keyboardPopup, keyboardPopupX, keyboardPopupY);
			keyboardIsDisplayed = true;
			break;

		case evInvertX:
			nvData.lcdOrientation = static_cast<DisplayOrientation>(nvData.lcdOrientation ^ (ReverseX | InvertBitmap));
			lcd.InitLCD(nvData.lcdOrientation, is24BitLcd);
			CalibrateTouch();
			CheckSettingsAreSaved();
			break;

		case evInvertY:
			nvData.lcdOrientation = static_cast<DisplayOrientation>(nvData.lcdOrientation ^ (ReverseX | ReverseY | InvertText | InvertBitmap));
			lcd.InitLCD(nvData.lcdOrientation, is24BitLcd);
			CalibrateTouch();
			CheckSettingsAreSaved();
			break;

		case evSetBaudRate:
			Adjusting(bp);
			mgr.SetPopup(baudPopup, fullWidthPopupX, popupY);
			break;

		case evAdjustBaudRate:
			nvData.baudRate = bp.GetIParam();
			SerialIo::Init(nvData.baudRate);
			baudRateButton->SetValue(nvData.baudRate);
			CheckSettingsAreSaved();
			CurrentButtonReleased();
			mgr.ClearPopup();
			StopAdjusting();
			break;

		case evSetVolume:
			Adjusting(bp);
			mgr.SetPopup(volumePopup, fullWidthPopupX, popupY);
			break;

		case evSetColours:
			if (coloursPopup != nullptr)
			{
				Adjusting(bp);
				mgr.SetPopup(coloursPopup, fullWidthPopupX, popupY);	
			}
			break;

		case evBrighter:
		case evDimmer:
			{
				int adjust = max<int>(1, (int)(nvData.brightness/16));
				if (ev == evDimmer)
				{
					adjust = -adjust;
				}
				nvData.brightness = min<int>(Buzzer::MaxBrightness, max<int>(Buzzer::MinBrightness, (int)nvData.brightness + adjust));
			}
			Buzzer::SetBacklight(nvData.brightness);
			CheckSettingsAreSaved();
			ShortenTouchDelay();
			break;
		
		case evAdjustVolume:
			nvData.touchVolume = bp.GetIParam();
			volumeButton->SetValue(nvData.touchVolume);
			TouchBeep();									// give audible feedback of the touch at the new volume level
			CheckSettingsAreSaved();
			break;

		case evAdjustColours:
			nvData.colourScheme = bp.GetIParam();
			coloursButton->SetText(colourSchemes[nvData.colourScheme].name);
			CheckSettingsAreSaved();
			break;

		case evSetLanguage:
			Adjusting(bp);
			mgr.SetPopup(languagePopup, fullWidthPopupX, popupY);
			break;

		case evAdjustLanguage:
			nvData.language = bp.GetIParam();
			languageButton->SetText(longLanguageNames[nvData.language]);
			CheckSettingsAreSaved();						// not sure we need this because we are going to reset anyway
			break;

		case evYes:
			CurrentButtonReleased();
			mgr.ClearPopup();								// clear the yes/no popup
			switch (eventToConfirm)
			{
			case evFactoryReset:
				FactoryReset();
				break;

			case evDeleteFile:
				if (currentFile != nullptr)
				{
					mgr.ClearPopup();						// clear the file info popup
					SerialIo::SendString("M30 ");
					SerialIo::SendFilename(StripPrefix(FileManager::GetFilesDir()), currentFile);
					SerialIo::SendChar('\n');
					FileManager::RefreshFilesList();
					currentFile = nullptr;
				}
				break;

			case evRestart:
				if (nvData != savedNvData)
				{
					SaveSettings();
				}
				Restart();
				break;

			default:
				break;
			}
			eventToConfirm = evNull;
			currentFile = nullptr;
			break;

		case evKey:
			if (!userCommandBuffers[currentUserCommandBuffer].full())
			{
				userCommandBuffers[currentUserCommandBuffer].add((char)bp.GetIParam());
				userCommandField->SetChanged();
			}
			break;

		case evBackspace:
			if (!userCommandBuffers[currentUserCommandBuffer].isEmpty())
			{
				userCommandBuffers[currentUserCommandBuffer].erase(userCommandBuffers[currentUserCommandBuffer].size() - 1);
				userCommandField->SetChanged();
				ShortenTouchDelay();
			}
			break;

		case evUp:
			currentHistoryBuffer = (currentHistoryBuffer + numUserCommandBuffers - 1) % numUserCommandBuffers;
			if (currentHistoryBuffer == currentUserCommandBuffer)
			{
				userCommandBuffers[currentUserCommandBuffer].clear();
			}
			else
			{
				userCommandBuffers[currentUserCommandBuffer].copy(userCommandBuffers[currentHistoryBuffer]);
			}
			userCommandField->SetChanged();
			break;

		case evDown:
			currentHistoryBuffer = (currentHistoryBuffer + 1) % numUserCommandBuffers;
			if (currentHistoryBuffer == currentUserCommandBuffer)
			{
				userCommandBuffers[currentUserCommandBuffer].clear();
			}
			else
			{
				userCommandBuffers[currentUserCommandBuffer].copy(userCommandBuffers[currentHistoryBuffer]);
			}
			userCommandField->SetChanged();
			break;

		case evSendKeyboardCommand:
			if (userCommandBuffers[currentUserCommandBuffer].size() != 0)
			{
				SerialIo::SendString(userCommandBuffers[currentUserCommandBuffer].c_str());
				SerialIo::SendChar('\n');
				
				// Add the command to the history if it was different frmo the previous command
				size_t prevBuffer = (currentUserCommandBuffer + numUserCommandBuffers - 1) % numUserCommandBuffers;
				if (strcmp(userCommandBuffers[currentUserCommandBuffer].c_str(), userCommandBuffers[prevBuffer].c_str()) != 0)
				{
					currentUserCommandBuffer = (currentUserCommandBuffer + 1) % numUserCommandBuffers;
				}
				currentHistoryBuffer = currentUserCommandBuffer;					
				userCommandBuffers[currentUserCommandBuffer].clear();
				userCommandField->SetLabel(userCommandBuffers[currentUserCommandBuffer].c_str());
			}
			break;

		default:
			break;
		}
	}
}

// Process a touch event outside the popup on the field being adjusted
void ProcessTouchOutsidePopup(ButtonPress bp)
pre(bp.IsValid())
{
	if (bp == fieldBeingAdjusted)
	{
		DelayTouchLong();	// by default, ignore further touches for a long time
		TouchBeep();
		switch(fieldBeingAdjusted.GetEvent())
		{
		case evAdjustSpeed:
		case evExtrusionFactor:
		case evAdjustFan:
			static_cast<IntegerButton*>(fieldBeingAdjusted.GetButton())->SetValue(oldIntValue);
			mgr.ClearPopup();
			StopAdjusting();
			break;

		case evAdjustActiveTemp:
		case evAdjustStandbyTemp:
		case evSetBaudRate:
		case evSetVolume:
		case evSetColours:
			mgr.ClearPopup();
			StopAdjusting();
			break;

		case evSetLanguage:
			mgr.ClearPopup();
			StopAdjusting();
			if (nvData.language != savedNvData.language)
			{
				restartNeeded = true;
				PopupRestart();
			}
			break;
		}
	}
	else
	{
		switch(bp.GetEvent())
		{
		case evTabControl:
		case evTabPrint:
		case evTabMsg:
		case evTabSetup:
			StopAdjusting();
			DelayTouchLong();	// by default, ignore further touches for a long time
			TouchBeep();
			ChangeTab(bp.GetButton());
			break;

		case evSetBaudRate:
		case evSetVolume:
		case evSetColours:
		case evSetLanguage:
		case evCalTouch:
		case evInvertX:
		case evInvertY:
		case evSaveSettings:
		case evFactoryReset:
		case evRestart:
			// On the Setup tab, we allow any other button to be pressed to exit the current popup
			StopAdjusting();
			DelayTouchLong();	// by default, ignore further touches for a long time
			TouchBeep();
			mgr.ClearPopup();
			ProcessTouch(bp);
			break;

		default:
			break;
		}
	}
}

// Update an integer field, provided it isn't the one being adjusted
// Don't update it if the value hasn't changed, because that makes the display flicker unnecessarily
void UpdateField(IntegerButton *f, int val)
{
	if (f != fieldBeingAdjusted.GetButton() && f->GetValue() != val)
	{
		f->SetValue(val);
	}
}

void UpdatePrintingFields()
{
	if (status == PrinterStatus::printing)
	{
		Fields::ShowPauseButton();
	}
	else if (status == PrinterStatus::paused)
	{
		Fields::ShowResumeAndCancelButtons();
	}
	else
	{
		Fields::ShowFilesButton();
	}
	
	mgr.Show(printProgressBar, PrintInProgress());
//	mgr.Show(printingField, PrintInProgress());

	// Don't enable the time left field when we start printing, instead this will get enabled when we receive a suitable message
	if (!PrintInProgress())
	{
		mgr.Show(timeLeftField, false);	
	}
	
	statusField->SetValue(statusText[(unsigned int)status]);
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
		gotGeometry = false;
		break;
	case 'D':
		newStatus = PrinterStatus::pausing;
		break;
	case 'F':
		newStatus = PrinterStatus::flashing;
		gotGeometry = false;
		break;
	case 'I':
		newStatus = PrinterStatus::idle;
		break;
	case 'P':
		newStatus = PrinterStatus::printing;
		break;
	case 'R':
		newStatus = PrinterStatus::resuming;
		break;
	case 'S':
		newStatus = PrinterStatus::stopped;
		gotGeometry = false;
		break;
	default:
		newStatus = status;		// leave the status alone if we don't recognize it
		break;
	}
	
	if (newStatus != status)
	{
		switch (newStatus)
		{
		case PrinterStatus::printing:
			if (status != PrinterStatus::paused && status != PrinterStatus::resuming)
			{
				// Starting a new print, so clear the times
				timesLeft[0] = timesLeft[1] = timesLeft[2] = 0;			
			}	
			// no break
		case PrinterStatus::paused:
		case PrinterStatus::pausing:
		case PrinterStatus::resuming:
			if (status == PrinterStatus::connecting || status == PrinterStatus::idle)
			{
				ChangeTab(tabPrint);
			}
			else if (currentTab == tabPrint)
			{
				nameField->SetValue(printingFile.c_str());
			}
			break;
			
		case PrinterStatus::idle:
			printingFile.clear();
			nameField->SetValue(machineName.c_str());		// if we are on the print tab then it may still be set to the file that was being printed
			// no break
		case PrinterStatus::configuring:
			if (status == PrinterStatus::flashing)
			{
				mgr.ClearAllPopups();						// clear the firmware update message
			}
			break;

		default:
			nameField->SetValue(machineName.c_str());
			break;
		}
		
		if (status == PrinterStatus::configuring || (status == PrinterStatus::connecting && newStatus != PrinterStatus::configuring))
		{
			MessageLog::AppendMessage("Connected");
			MessageLog::DisplayNewMessage();
		}
	
		status = newStatus;
		UpdatePrintingFields();
	}
}

// Append an amount of time to timesLeftText
void AppendTimeLeft(int t)
{
	if (t <= 0)
	{
		timesLeftText.catFrom("n/a");
	}
	else if (t < 60)
	{
		timesLeftText.catf("%ds", t);
	}
	else if (t < 60 * 60)
	{
		timesLeftText.catf("%dm %02ds", t/60, t%60);
	}
	else
	{
		t /= 60;
		timesLeftText.catf("%dh %02dm", t/60, t%60);
	}
}

// Try to get an integer value from a string. If it is actually a floating point value, round it.
bool GetInteger(const char s[], int &rslt)
{
	if (s[0] == 0) return false;			// empty string

	char* endptr;
	rslt = (int) strtol(s, &endptr, 10);
	if (*endptr == 0) return true;			// we parsed an integer

	if (strlen(s) > 10) return false;		// avoid strtod buggy behaviour on long input strings

	double d = strtod(s, &endptr);			// try parsing a floating point number
	if (*endptr == 0)
	{
		rslt = (int)((d < 0.0) ? d - 0.5 : d + 0.5);
		return true;
	}
	return false;
}

// Try to get an unsigned integer value from a string
bool GetUnsignedInteger(const char s[], unsigned int &rslt)
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

// These tables must be kept in alphabetical order of the search string
const ReceiveDataTableEntry arrayDataTable[] =
{
	{ rcvActive,		"active" },
	{ rcvEfactor,		"efactor" },
	{ rcvFanPercent,	"fanPercent" },
	{ rcvFilament,		"filament" },
	{ rcvFiles,			"files" },
	{ rcvHeaters,		"heaters" },
	{ rcvHomed,			"homed" },
	{ rcvHstat,			"hstat" },
	{ rcvPos,			"pos" },
	{ rcvStandby,		"standby" },
	{ rcvTimesLeft,		"timesLeft" }
};

const ReceiveDataTableEntry nonArrayDataTable[] =
{
	{ rcvAxes,			"axes" },
	{ rcvBeepFreq,		"beep_freq" },
	{ rcvBeepLength,	"beep_length" },
	{ rcvDir,			"dir" },
	{ rcvErr,			"err" },
	{ rcvFilename,		"fileName" },
	{ rcvFraction,		"fraction_printed" },
	{ rcvGeneratedBy,	"generatedBy" },
	{ rcvGeometry,		"geometry" },
	{ rcvHeight,		"height" },
	{ rcvLayerHeight,	"layerHeight" },
	{ rcvMessage,		"message" },
	{ rcvMyName,		"myName" },
	{ rcvProbe,			"probe" },
	{ rcvResponse,		"resp" },
	{ rcvSeq,			"seq" },
	{ rcvSfactor,		"sfactor" },
	{ rcvSize,			"size" },
	{ rcvStatus,		"status" },
	{ rcvVolumes,		"volumes" }
};

void StartReceivedMessage()
{
	ShowLine;
	newMessageSeq = messageSeq;
	MessageLog::BeginNewMessage();
	FileManager::BeginNewMessage();
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
	FileManager::EndReceivedMessage(currentFile != nullptr);	
	ShowLine;
}

// Public functions called by the SerialIo module
void ProcessReceivedValue(const char id[], const char data[], int index)
{
	if (index >= 0)			// if this is an element of an array
	{
		ShowLine;
		switch(bsearch(arrayDataTable, sizeof(arrayDataTable)/sizeof(arrayDataTable[0]), id))
		{
		case rcvActive:
			ShowLine;
			{
				int ival;
				if (GetInteger(data, ival) && index < (int)maxHeaters)
				{
					UpdateField(activeTemps[index], ival);
				}
			}
			break;

		case rcvStandby:
			ShowLine;
			{
				int ival;
				if (GetInteger(data, ival) && index < (int)maxHeaters && index != 0)
				{
					UpdateField(standbyTemps[index], ival);
				}
			}
			break;
		
		case rcvHeaters:
			ShowLine;
			{
				float fval;
				if (GetFloat(data, fval) && index < (int)maxHeaters)
				{
					ShowLine;
					currentTemps[index]->SetValue(fval);
					if (index == (int)numHeads + 1)
					{
						ShowLine;
						mgr.Show(currentTemps[index], true);
						mgr.Show(activeTemps[index], true);
						mgr.Show(standbyTemps[index], true);
						mgr.Show(extrusionFactors[index - 1], true);
						++numHeads;
					}
				}
			}
			break;

		case rcvHstat:
			ShowLine;
			{
				int ival;
				if (GetInteger(data, ival) && index < (int)maxHeaters)
				{
					heaterStatus[index] = ival;
					Colour c = (ival == 1) ? colours->standbyBackColour
								: (ival == 2) ? colours->activeBackColour
								: (ival == 3) ? colours->errorBackColour
								: (ival == 4) ? colours->tuningBackColour
								: colours->defaultBackColour;
					currentTemps[index]->SetColours((ival == 3) ? colours->errorTextColour : colours->infoTextColour, c);
				}
			}
			break;
			
		case rcvPos:
			ShowLine;
			{
				float fval;
				if (GetFloat(data, fval) && index < MAX_AXES)
				{
					axisPos[index]->SetValue(fval);
				}
			}
			break;
		
		case rcvEfactor:
			ShowLine;
			{
				int ival;
				if (GetInteger(data, ival) && index + 1 < (int)maxHeaters)
				{
					UpdateField(extrusionFactors[index], ival);
				}
			}
			break;
		
		case rcvFiles:
			ShowLine;
			if (index == 0)
			{
				FileManager::BeginReceivingFiles();
			}
			FileManager::ReceiveFile(data);
			break;
		
		case rcvFilament:
			ShowLine;
			{
				static float totalFilament = 0.0;
				if (index == 0)
				{
					totalFilament = 0.0;
				}
				float f;
				if (GetFloat(data, f))
				{
					totalFilament += f;
					fpFilamentField->SetValue((int)totalFilament);
				}
			}
			break;
		
		case rcvHomed:
			ShowLine;
			{
				int ival;
				if (index < MAX_AXES && GetInteger(data, ival) && ival >= 0 && ival < 2)
				{
					bool isHomed = (ival == 1);
					if (isHomed != axisHomed[index])
					{
						axisHomed[index] = isHomed;
						homeButtons[index]->SetColours(colours->buttonTextColour, (isHomed) ? colours->homedButtonBackColour : colours->notHomedButtonBackColour);
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
							homeAllButton->SetColours(colours->buttonTextColour, (allAxesHomed) ? colours->homedButtonBackColour : colours->notHomedButtonBackColour);
						}
					}
				}
			}
			break;
		
		case rcvTimesLeft:
			ShowLine;
			if (index < (int)ARRAY_SIZE(timesLeft))
			{
				int i;
				bool b = GetInteger(data, i);
				if (b && i >= 0 && i < 10 * 24 * 60 * 60 && PrintInProgress())
				{
					timesLeft[index] = i;
					timesLeftText.copy("file ");
					AppendTimeLeft(timesLeft[0]);
					timesLeftText.catFrom(", filament ");
					AppendTimeLeft(timesLeft[1]);
					if (DisplayX >= 800)
					{
						timesLeftText.catFrom(", layer ");
						AppendTimeLeft(timesLeft[2]);
					}
					timeLeftField->SetValue(timesLeftText.c_str());
					mgr.Show(timeLeftField, true);
				}
			}
			break;

		case rcvFanPercent:
			ShowLine;
			if (index == 0)			// currently we only handle one fan
			{
				float f;
				bool b = GetFloat(data, f);
				if (b && f >= 0.0 && f <= 100.0)
				{
					UpdateField(fanSpeed, (int)(f + 0.5));
				}
			}
			break;

		default:
			break;
		}
	}
	else
	{
		ShowLine;
		// Non-array values follow
		switch(bsearch(nonArrayDataTable, sizeof(nonArrayDataTable)/sizeof(nonArrayDataTable[0]), id))
		{
		case rcvSfactor:
			{
				int ival;
				if (GetInteger(data, ival))
				{
					UpdateField(spd, ival);
				}
			}
			break;

		case rcvProbe:
			zprobeBuf.copy(data);
			zProbe->SetChanged();
			break;
		
		case rcvMyName:
			if (status != PrinterStatus::configuring && status != PrinterStatus::connecting)
			{
				machineName.copy(data);
				nameField->SetChanged();
				gotMachineName = true;
				if (gotGeometry)
				{
					machineConfigTimer.Stop();
				}
			}
			break;
		
		case rcvFilename:
			if (!printingFile.similar(data))
			{
				printingFile.copy(data);
				if (currentTab == tabPrint && PrintInProgress())
				{
					nameField->SetChanged();
				}
			}
			fileInfoTimer.Stop();
			break;
		
		case rcvSize:
			{
				int sz;
				if (GetInteger(data, sz))
				{
					fpSizeField->SetValue(sz);
				}
			}
			break;
		
		case rcvHeight:
			{
				float f;
				if (GetFloat(data, f))
				{
					fpHeightField->SetValue(f);
				}
			}
			break;
		
		case rcvLayerHeight:
			{
				float f;
				if (GetFloat(data, f))
				{
					fpLayerHeightField->SetValue(f);
				}
			}
			break;
		
		case rcvGeneratedBy:
			generatedByText.copy(data);
			fpGeneratedByField->SetChanged();
			break;
		
		case rcvFraction:
			{
				float f;
				if (GetFloat(data, f))
				{
					if (f >= 0.0 && f <= 1.0)
					{
						printProgressBar->SetPercent((uint8_t)((100.0 * f) + 0.5));
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
				gotGeometry = true;
				if (gotMachineName)
				{
					machineConfigTimer.Stop();
				}
				for (size_t i = 0; i < MAX_AXES; ++i)
				{
					mgr.Show(homeButtons[i], !isDelta && i < numAxes);
				}
			}
			break;
		
		case rcvAxes:
			{
				unsigned int n = MIN_AXES;
				GetUnsignedInteger(data, n);
				numAxes = constrain<unsigned int>(n, MIN_AXES, MAX_AXES);
				for (size_t i = MIN_AXES; i < MAX_AXES; ++i)
				{
					mgr.Show(homeButtons[i], !isDelta && i < numAxes);
					Fields::ShowAxis(i, i < numAxes);
				}
			}
			break;

		case rcvSeq:
			GetUnsignedInteger(data, newMessageSeq);
			break;
		
		case rcvResponse:
			MessageLog::AppendMessage(data);
			break;
		
		case rcvDir:
			FileManager::ReceiveDirectoryName(data);
			break;

		case rcvMessage:
			if (data[0] == 0)
			{
				mgr.ClearPopup(true, alertPopup);
			}
			else
			{
				alertText.copy(data);
				mgr.SetPopup(alertPopup, (DisplayX - alertPopupWidth)/2, (DisplayY - alertPopupHeight)/2);
			}
			break;

		case rcvErr:
			{
				int i;
				if (GetInteger(data, i))
				{
					FileManager::ReceiveErrorCode(i);
				}
			}
			break;

		case rcvVolumes:
			{
				unsigned int i;
				if (GetUnsignedInteger(data, i))
				{
					FileManager::SetNumVolumes(i);
				}
			}
			break;

		default:
			break;
		}
	}
	ShowLine;
}

// Public function called when the serial I/O module finishes receiving an array of values
void ProcessArrayLength(const char id[], int length)
{
	if (length == 0 && strcmp(id, "files") == 0)
	{
		FileManager::BeginReceivingFiles();				// received an empty file list - need to tell the file manager about it
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

void SendRequest(const char *s, bool includeSeq = false)
{
	SerialIo::SendString(s);
	if (includeSeq)
	{
		SerialIo::SendInt(messageSeq);
	}
	SerialIo::SendChar('\n');
	lastPollTime = SystemTick::GetTickCount();
}

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
	pmc_enable_periph_clk(ID_UART1);	// enable UART1 clock
	
	Buzzer::Init();						// init the buzzer, must also call this before the backlight can be used

	wdt_init (WDT, WDT_MR_WDRSTEN, 1000, 1000);
	SysTick_Config(SystemCoreClock / 1000);
	lastTouchTime = SystemTick::GetTickCount();


	// Read parameters from flash memory
	nvData.Load();
	if (nvData.IsValid())
	{
		// The touch panel has already been calibrated
		InitLcd(nvData.lcdOrientation, nvData.language, nvData.colourScheme);
		touch.init(DisplayX, DisplayY, nvData.touchOrientation);
		touch.calibrate(nvData.xmin, nvData.xmax, nvData.ymin, nvData.ymax, touchCalibMargin);
		savedNvData = nvData;
		Buzzer::SetBacklight(nvData.brightness);
	}
	else
	{
		// The touch panel has not been calibrated, and we do not know which way up it is
		nvData.SetDefaults();
		InitLcd(nvData.lcdOrientation, nvData.language, nvData.colourScheme);
		Buzzer::SetBacklight(nvData.brightness);	// must be done before touch calibration
		CalibrateTouch();							// this includes the touch driver initialization
		SaveSettings();
	}
	
	// Set up the baud rate
	SerialIo::Init(nvData.baudRate);
	baudRateButton->SetValue(nvData.baudRate);
	volumeButton->SetValue(nvData.touchVolume);
	coloursButton->SetText(colourSchemes[nvData.colourScheme].name);
	
	MessageLog::Init();

	UpdatePrintingFields();

	lastPollTime = SystemTick::GetTickCount() - printerPollInterval;	// allow a poll immediately
	
	// Hide the Head 2+ parameters until we know we have a second head
	for (unsigned int i = 2; i < maxHeaters; ++i)
	{
		currentTemps[i]->Show(false);
		activeTemps[i]->Show(false);
		standbyTemps[i]->Show(false);
		extrusionFactors[i - 1]->Show(false);
	}
	
	standbyTemps[0]->Show(false);			// currently, we always hide the bed standby temperature because it doesn't do anything
	debugField->Show(DEBUG != 0);			// show the debug field only if debugging is enabled

	userCommandField->SetLabel(userCommandBuffers[currentUserCommandBuffer].c_str());	// set up to display the current user command
	
#ifdef OEM
	// Display the splash screen
	lcd.drawCompressedBitmap(0, 0, DISPLAY_X, DISPLAY_Y, splashScreenImage);
	const uint32_t now = SystemTick::GetTickCount();
	while (SystemTick::GetTickCount() - now < 5000) { }		// hold it there for 5 seconds
#endif

	// Display the Control tab. This also refreshes the display.
	ChangeTab(tabControl);
	lastResponseTime = SystemTick::GetTickCount();		// pretend we just received a response
	
	machineConfigTimer.SetPending();		// we need to fetch the machine name and configuration

	for (;;)
	{
		ShowLine;

		// 1. Check for input from the serial port and process it.
		// This calls back into functions StartReceivedMessage, ProcessReceivedValue, ProcessArrayLength and EndReceivedMessage.
		SerialIo::CheckInput();
		ShowLine;
		
		// 2. if displaying the message log, update the times
		if (currentTab == tabMsg)
		{
			MessageLog::UpdateMessages(false);
		}
		ShowLine;
		
		// 3. Check for a touch on the touch panel.
		if (SystemTick::GetTickCount() - lastTouchTime >= ignoreTouchTime)
		{
			if (currentButton.IsValid())
			{
				CurrentButtonReleased();
			}

			uint16_t x, y;
			if (touch.read(x, y))
			{
#if DEBUG
				touchX->SetValue((int)x);	//debug
				touchY->SetValue((int)y);	//debug
#endif
				ButtonPress bp = mgr.FindEvent(x, y);
				if (bp.IsValid())
				{
					DelayTouchLong();		// by default, ignore further touches for a long time
					if (bp.GetEvent() != evAdjustVolume)
					{
						TouchBeep();		// give audible feedback of the touch, unless adjusting the volume	
					}
					ProcessTouch(bp);
				}
				else
				{
					bp = mgr.FindEventOutsidePopup(x, y);
					if (bp.IsValid())
					{
						ProcessTouchOutsidePopup(bp);					
					}
				}
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
		uint32_t now = SystemTick::GetTickCount();
		if (   currentTab != tabSetup								// don't poll while we are in the Setup page
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
				if (!done)
				{
					done = fileInfoTimer.Process();
				}
				
				// Otherwise just send a normal poll command
				if (!done)
				{
					SendRequest("M408 S0 R", true);					// normal poll response
				}
			}
			else if (now - lastPollTime >= printerPollTimeout)		// if we're giving up on getting a response to the last poll
			{
				SendRequest("M408 S0");								// just send a normal poll message, don't ask for the last response
			}
		}
		ShowLine;
	}
}

void PrintDebugText(const char *x)
{
	fwVersionField->SetValue(x);
}

// End
