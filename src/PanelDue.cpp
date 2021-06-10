// TFT panel controller to run on SAM3S2B/SAM3S4B/SAM4S4B
// Coding rules:
//
// 1. Must compile with no g++ warnings, when all warning are enabled.
// 2. Dynamic memory allocation using 'new' is permitted during the initialization phase only. No use of 'new' anywhere else,
//    and no use of 'delete', 'malloc' or 'free' anywhere.
// 3. No pure virtual functions. This is because in release builds, having pure virtual functions causes huge amounts of the C++ library to be linked in
//    (possibly because it wants to print a message if a pure virtual function is called).

#include "PanelDue.hpp"
#include "asf.h"

#include <cstring>
#include <cctype>
#include <cmath>

#include <Hardware/Mem.hpp>
#include <Hardware/UTFT.hpp>
#include <Hardware/UTouch.hpp>
#include <Hardware/SerialIo.hpp>
#include <Hardware/Buzzer.hpp>
#include <Hardware/SysTick.hpp>
#include <Hardware/Reset.hpp>
#include <General/SafeStrtod.h>
#include <General/SimpleMath.h>
#include <General/StringFunctions.h>

#include "Configuration.hpp"
#include <UI/UserInterfaceConstants.hpp>
#include "FileManager.hpp"
#include <UI/MessageLog.hpp>
#include <UI/Events.hpp>
#include <UI/UserInterface.hpp>
#include <ObjectModel/Axis.hpp>
#include <ObjectModel/PrinterStatus.hpp>
#include "ControlCommands.hpp"

extern uint16_t _esplash[];							// defined in linker script

#define DEBUG	(0) // 1: MessageLog only, 2: DebugField only, 3: MessageLog & DebugField

#if (DEBUG & 1)
#define dbg(fmt, args...)		do { MessageLog::AppendMessageF("%s(%d): " fmt , __FUNCTION__, __LINE__, ##args); } while(0)

#else
#define dbg(fmt, args...)		do {} while(0)

#endif

#if (DEBUG & (1 << 1))

#define STRINGIFY(x)	#x
#define TOSTRING(x)	STRINGIFY(x)
#define dbg2(fmt, args...)		debugField->SetValue(TOSTRING(__LINE__)); debugField->Refresh(true, 0, 0)

#else
#define dbg2(fmt, args...)		do {} while(0)

#endif

// Controlling constants
constexpr uint32_t defaultPrinterPollInterval = 500;	// poll interval in milliseconds
constexpr uint32_t defaultPrinterResponseInterval = defaultPrinterPollInterval * 0.7;		// shortest time after a response that we send another poll (gives printer time to catch up)
constexpr uint32_t slowPrinterPollInterval = 4000;		// poll interval in milliseconds when screensaver active
const uint32_t printerPollTimeout = 2000;			// poll timeout in milliseconds
const uint32_t FileInfoRequestTimeout = 8000;		// file info request timeout in milliseconds
const uint32_t touchBeepLength = 20;				// beep length in ms
const uint32_t touchBeepFrequency = 4500;			// beep frequency in Hz. Resonant frequency of the piezo sounder is 4.5kHz.
const uint32_t errorBeepLength = 100;
const uint32_t errorBeepFrequency = 2250;
const uint32_t longTouchDelay = 250;				// how long we ignore new touches for after pressing Set
const uint32_t shortTouchDelay = 100;				// how long we ignore new touches while pressing up/down, to get a reasonable repeat rate

struct HostFirmwareType
{
	const char* _ecv_array const name;
	FirmwareFeatureMap features;
};

const HostFirmwareType firmwareTypes[] =
{
	{ "RepRapFirmware", FirmwareFeatureMap::MakeFromRaw(1 << quoteFilenames) },
	{ "Smoothie", 		FirmwareFeatureMap::MakeFromRaw(1 << noGcodesFolder | 1 << noStandbyTemps | 1 << noG10Temps | 1 << noDriveNumber | 1 << noM20M36) },
	{ "Repetier", 		FirmwareFeatureMap::MakeFromRaw(1 << noGcodesFolder | 1 << noStandbyTemps | 1 << noG10Temps) },
	{ "Marlin",			FirmwareFeatureMap::MakeFromRaw(1 << noGcodesFolder | 1 << noStandbyTemps | 1 << noG10Temps) }
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

// These defines control which detailed M409 requests will be sent
// If one of the fields in the disabled ones need to be fetched the
// corresponding define has to be set to (1)
#define FETCH_BOARDS		(1)
#define FETCH_DIRECTORIES	(0)
#define FETCH_FANS			(0)
#define FETCH_HEAT			(1)
#define FETCH_INPUTS		(0)
#define FETCH_JOB			(1)
#define FETCH_MOVE			(1)
#define FETCH_NETWORK		(1)
#define FETCH_SCANNER		(0)
#define FETCH_SENSORS		(0)
#define FETCH_SPINDLES		(1)
#define FETCH_STATE			(1)
#define FETCH_TOOLS			(1)
#define FETCH_VOLUMES		(1)

MainWindow mgr;

static uint32_t lastTouchTime;
static uint32_t ignoreTouchTime;
static uint32_t lastPollTime = 0;
static uint32_t lastResponseTime = 0;
static uint32_t lastOutOfBufferResponse = 0;
static uint8_t oobCounter = 0;
static bool outOfBuffers = false;
static uint32_t lastActionTime = 0;							// the last time anything significant happened
static FirmwareFeatureMap firmwareFeatures;
static bool isDimmed = false;								// true if we have dimmed the display
static bool screensaverActive = false;						// true if screensaver is active
static bool isDelta = false;
static size_t numAxes = MIN_AXES;
static int32_t beepFrequency = 0, beepLength = 0;
static uint32_t messageSeq = 0;
static uint32_t newMessageSeq = 0;
static uint32_t fileSize = 0;
static uint8_t visibleAxesCounted = 0;
static int8_t lastBed = -1;
static int8_t lastChamber = -1;
static int8_t lastSpindle = -1;
static int8_t lastTool = -1;
static uint8_t mountedVolumesCounted = 0;
static uint32_t remoteUpTime = 0;
static bool initialized = false;
static float pollIntervalMultiplier = 1.0;
static uint32_t printerPollInterval = defaultPrinterPollInterval;
static uint32_t printerResponseInterval = defaultPrinterResponseInterval;

static const ColourScheme *colours = &colourSchemes[0];

Alert currentAlert;
uint32_t lastAlertSeq = 0;

static OM::PrinterStatus status = OM::PrinterStatus::connecting;

FlashData nvData, savedNvData;

enum ReceivedDataEvent
{
	rcvUnknown = 0,

	// Keys for control command messages
	rcvControlCommand,

	// Keys for push messages
	rcvPushMessage,
	rcvPushResponse,
	rcvPushSeq,
	rcvPushBeepDuration,
	rcvPushBeepFrequency,

	// Keys for M20 response
	rcvM20Dir,
	rcvM20Err,
	rcvM20Files,

	// Keys for M36 respons
	rcvM36Filament,
	rcvM36Filename,
	rcvM36GeneratedBy,
	rcvM36Height,
	rcvM36LastModified,
	rcvM36LayerHeight,
	rcvM36PrintTime,
	rcvM36SimulatedTime,
	rcvM36Size,

	// Keys for M409 response
	rcvKey,
	rcvFlags,
	rcvResult,

	// Available keys
	rcvOMKeyBoards,
	rcvOMKeyDirectories,
	rcvOMKeyFans,
	rcvOMKeyHeat,
	rcvOMKeyInputs,
	rcvOMKeyJob,
	rcvOMKeyLimits,
	rcvOMKeyMove,
	rcvOMKeyNetwork,
	rcvOMKeyReply,
	rcvOMKeyScanner,
	rcvOMKeySensors,
	rcvOMKeySeqs,
	rcvOMKeySpindles,
	rcvOMKeyState,
	rcvOMKeyTools,
	rcvOMKeyVolumes,

	// Keys for boards response
	rcvBoardsFirmwareName,

	// Keys for fans response
	rcvFansRequestedValue,

	// Keys for heat response
	rcvHeatBedHeaters,
	rcvHeatChamberHeaters,
	rcvHeatHeatersActive,
	rcvHeatHeatersCurrent,
	rcvHeatHeatersStandby,
	rcvHeatHeatersState,

	// Keys for job response
	rcvJobDuration,
	rcvJobFileFilename,
	rcvJobFileSize,
	rcvJobFilePosition,
	rcvJobFileSimulatedTime,
	rcvJobLastFileName,
	rcvJobLastFileSimulated,
	rcvJobTimesLeftFilament,
	rcvJobTimesLeftFile,
	rcvJobTimesLeftSlicer,
	rcvJobWarmUpDuration,

	// Keys for move response
	rcvMoveAxesBabystep,
	rcvMoveAxesHomed,
	rcvMoveAxesLetter,
	rcvMoveAxesMachinePosition,
	rcvMoveAxesUserPosition,
	rcvMoveAxesVisible,
	rcvMoveAxesWorkplaceOffsets,
	rcvMoveExtrudersFactor,
	rcvMoveKinematicsName,
	rcvMoveSpeedFactor,
	rcvMoveWorkplaceNumber,

	// Keys for network response
	rcvNetworkName,
	rcvNetworkInterfacesActualIP,

	// Keys for sensors response
	rcvSensorsProbeValue,

	// Keys for seqs response
	rcvSeqsBoards,
	rcvSeqsDirectories,
	rcvSeqsFans,
	rcvSeqsHeat,
	rcvSeqsInputs,
	rcvSeqsJob,
	rcvSeqsMove,
	rcvSeqsNetwork,
	rcvSeqsReply,
	rcvSeqsScanner,
	rcvSeqsSensors,
	rcvSeqsSpindles,
	rcvSeqsState,
	rcvSeqsTools,
	rcvSeqsVolumes,

	// Keys for spindles respons
	rcvSpindlesActive,
	rcvSpindlesCurrent,
	rcvSpindlesMax,
	rcvSpindlesMin,
	rcvSpindlesState,
	rcvSpindlesTool,

	// Keys from state response
	rcvStateCurrentTool,
	rcvStateMessageBox,
	rcvStateMessageBoxAxisControls,
	rcvStateMessageBoxMessage,
	rcvStateMessageBoxMode,
	rcvStateMessageBoxSeq,
	rcvStateMessageBoxTimeout,
	rcvStateMessageBoxTitle,
	rcvStateStatus,
	rcvStateUptime,

	// Keys from tools response
	rcvToolsActive,
	rcvToolsExtruders,
	rcvToolsFans,
	rcvToolsHeaters,
	rcvToolsOffsets,
	rcvToolsNumber,
	rcvToolsSpindle,
	rcvToolsSpindleRpm,
	rcvToolsStandby,
	rcvToolsState,

	// Keys for volumes response
	rcvVolumesMounted,
};

struct FieldTableEntry
{
	const ReceivedDataEvent val;
	const char* key;
};

// The following tables will be sorted once on startup so entries can be better grouped for code maintenance
// A '^' character indicates the position of an _ecv_array index, and a ':' character indicates the start of a sub-field name
static FieldTableEntry fieldTable[] =
{
	// M409 common fields
	{ rcvKey, 							"key" },

	// M409 K"boards" response
	{ rcvBoardsFirmwareName, 			"boards^:firmwareName" },

	// M409 K"fans" response
	{ rcvFansRequestedValue,			"fans^:requestedValue" },

	// M409 K"heat" response
	{ rcvHeatBedHeaters,				"heat:bedHeaters^" },
	{ rcvHeatChamberHeaters,			"heat:chamberHeaters^" },
	{ rcvHeatHeatersActive,				"heat:heaters^:active" },
	{ rcvHeatHeatersCurrent,			"heat:heaters^:current" },
	{ rcvHeatHeatersStandby,			"heat:heaters^:standby" },
	{ rcvHeatHeatersState,				"heat:heaters^:state" },

	// M409 K"job" response
	{ rcvJobFileFilename, 				"job:file:fileName" },
	{ rcvJobFileSize, 					"job:file:size" },
	{ rcvJobFileSimulatedTime, 			"job:file:simulatedTime" },
	{ rcvJobFilePosition,				"job:filePosition" },
	{ rcvJobLastFileName,				"job:lastFileName" },
	{ rcvJobLastFileSimulated,			"job:lastFileSimulated" },
	{ rcvJobTimesLeftFilament,			"job:timesLeft:filament" },
	{ rcvJobTimesLeftFile,				"job:timesLeft:file" },
	{ rcvJobTimesLeftSlicer,			"job:timesLeft:slicer" },
	{ rcvJobWarmUpDuration,				"job:warmUpDuration" },

	// M409 K"move" response
	{ rcvMoveAxesBabystep, 				"move:axes^:babystep" },
	{ rcvMoveAxesHomed,					"move:axes^:homed" },
	{ rcvMoveAxesLetter,	 			"move:axes^:letter" },
	{ rcvMoveAxesMachinePosition,		"move:axes^:machinePosition" },
	{ rcvMoveAxesUserPosition,			"move:axes^:userPosition" },
	{ rcvMoveAxesVisible, 				"move:axes^:visible" },
	{ rcvMoveAxesWorkplaceOffsets, 		"move:axes^:workplaceOffsets^" },
	{ rcvMoveExtrudersFactor, 			"move:extruders^:factor" },
	{ rcvMoveKinematicsName, 			"move:kinematics:name" },
	{ rcvMoveSpeedFactor, 				"move:speedFactor" },
	{ rcvMoveWorkplaceNumber, 			"move:workplaceNumber" },

	// M409 K"network" response
	{ rcvNetworkName, 					"network:name" },
	{ rcvNetworkInterfacesActualIP,		"network:interfaces^:actualIP" },

	// M409 K"sensors" response
	{ rcvSensorsProbeValue,				"sensors:probes^:value^" },

	// M409 K"seqs" response
	{ rcvSeqsBoards,					"seqs:boards" },
	{ rcvSeqsDirectories,				"seqs:directories" },
	{ rcvSeqsFans,						"seqs:fans" },
	{ rcvSeqsHeat,						"seqs:heat" },
	{ rcvSeqsInputs,					"seqs:inputs" },
	{ rcvSeqsJob,						"seqs:job" },
	{ rcvSeqsMove,						"seqs:move" },
	{ rcvSeqsNetwork,					"seqs:network" },
	{ rcvSeqsReply,						"seqs:reply" },
	{ rcvSeqsScanner,					"seqs:scanner" },
	{ rcvSeqsSensors,					"seqs:sensors" },
	{ rcvSeqsSpindles,					"seqs:spindles" },
	{ rcvSeqsState,						"seqs:state" },
	{ rcvSeqsTools,						"seqs:tools" },
	{ rcvSeqsVolumes,					"seqs:volumes" },

	// M409 K"spindles" response
	{ rcvSpindlesActive, 				"spindles^:active" },
	{ rcvSpindlesCurrent,				"spindles^:current" },
	{ rcvSpindlesMax, 					"spindles^:max" },
	{ rcvSpindlesMin, 					"spindles^:min" },
	{ rcvSpindlesState, 				"spindles^:state" },
	{ rcvSpindlesTool,	 				"spindles^:tool" },

	// M409 K"state" response
	{ rcvStateCurrentTool,				"state:currentTool" },
	{ rcvStateMessageBox,				"state:messageBox" },
	{ rcvStateMessageBoxAxisControls,	"state:messageBox:axisControls" },
	{ rcvStateMessageBoxMessage,		"state:messageBox:message" },
	{ rcvStateMessageBoxMode,			"state:messageBox:mode" },
	{ rcvStateMessageBoxSeq,			"state:messageBox:seq" },
	{ rcvStateMessageBoxTimeout,		"state:messageBox:timeout" },
	{ rcvStateMessageBoxTitle,			"state:messageBox:title" },
	{ rcvStateStatus,					"state:status" },
	{ rcvStateUptime,					"state:upTime" },

	// M409 K"tools" response
	{ rcvToolsActive, 					"tools^:active^" },
	{ rcvToolsExtruders,				"tools^:extruders^" },
	{ rcvToolsFans,						"tools^:fans^" },
	{ rcvToolsHeaters,					"tools^:heaters^" },
	{ rcvToolsNumber, 					"tools^:number" },
	{ rcvToolsOffsets, 					"tools^:offsets^" },
	{ rcvToolsSpindle, 					"tools^:spindle" },
	{ rcvToolsSpindleRpm,				"tools^:spindleRpm" },
	{ rcvToolsStandby, 					"tools^:standby^" },
	{ rcvToolsState, 					"tools^:state" },

	// M409 K"volumes" response
	{ rcvVolumesMounted, 				"volumes^:mounted" },

	// M20 response
	{ rcvM20Dir,						"dir" },
	{ rcvM20Err,						"err" },
	{ rcvM20Files,						"files^" },

	// M36 response
	{ rcvM36Filament,					"filament^" },
	{ rcvM36Filename,					"fileName" },
	{ rcvM36GeneratedBy,				"generatedBy" },
	{ rcvM36Height,						"height" },
	{ rcvM36LastModified,				"lastModified" },
	{ rcvM36LayerHeight,				"layerHeight" },
	{ rcvM36PrintTime,					"printTime" },
	{ rcvM36SimulatedTime,				"simulatedTime" },
	{ rcvM36Size,						"size" },

	// Push messages
	{ rcvPushMessage,					"message" },
	{ rcvPushResponse,					"resp" },
	{ rcvPushSeq,						"seq" },
	{ rcvPushBeepDuration,				"beep_length" },
	{ rcvPushBeepFrequency,				"beep_freq" },

	// Control Command message
	{ rcvControlCommand,				"controlCommand" },
};

enum SeqState {
	SeqStateInit,
	SeqStateOk,
	SeqStateUpdate,
	SeqStateError,
	SeqStateDisabled
};

static struct Seq {
	const ReceivedDataEvent event;
	const ReceivedDataEvent seqid;

	uint16_t lastSeq;
	enum SeqState state;

	const char * _ecv_array const key;
	const char * _ecv_array const flags;
} seqs[] = {
#if FETCH_NETWORK
	{ .event = rcvOMKeyNetwork, .seqid = rcvSeqsNetwork, .lastSeq = 0, .state = SeqStateInit, .key = "network", .flags = "v" },
#endif
#if FETCH_BOARDS
	{ .event = rcvOMKeyBoards, .seqid = rcvSeqsBoards, .lastSeq = 0, .state = SeqStateInit, .key = "boards", .flags = "v" },
#endif
#if FETCH_MOVE
	{ .event = rcvOMKeyMove, .seqid = rcvSeqsMove, .lastSeq = 0, .state = SeqStateInit, .key = "move", .flags = "v" },
#endif
#if FETCH_HEAT
	{ .event = rcvOMKeyHeat, .seqid = rcvSeqsHeat, .lastSeq = 0, .state = SeqStateInit, .key = "heat", .flags = "v" },
#endif
#if FETCH_TOOLS
	{ .event = rcvOMKeyTools, .seqid = rcvSeqsTools, .lastSeq = 0, .state = SeqStateInit, .key = "tools", .flags = "v" },
#endif
#if FETCH_SPINDLES
	{ .event = rcvOMKeySpindles, .seqid = rcvSeqsSpindles, .lastSeq = 0, .state = SeqStateInit, .key = "spindles", .flags = "v" },
#endif
#if FETCH_DIRECTORIES
	{ .event = rcvOMKeyDirectories, .seqid = rcvSeqsDirectories, .lastSeq = 0, .state = SeqStateInit, .key = "directories", .flags = "v" },
#endif
#if FETCH_FANS
	{ .event = rcvOMKeyFans, .seqid = rcvSeqsFans, .lastSeq = 0, .state = SeqStateInit, .key = "fans", .flags = "v" },
#endif
#if FETCH_INPUTS
	{ .event = rcvOMKeyInputs, .seqid = rcvSeqsInputs, .lastSeq = 0, .state = SeqStateInit, .key = "inputs", .flags = "v" },
#endif
#if FETCH_JOB
	{ .event = rcvOMKeyJob, .seqid = rcvSeqsJob, .lastSeq = 0, .state = SeqStateInit, .key = "job", .flags = "v" },
#endif
#if FETCH_SCANNER
	{ .event = rcvOMKeyScanner, .seqid = rcvSeqsScanner, .lastSeq = 0, .state = SeqStateInit, .key = "scanner", .flags = "v" },
#endif
#if FETCH_SENSORS
	{ .event = rcvOMKeySensors, .seqid = rcvSeqsSensors, .lastSeq = 0, .state = SeqStateInit, .key = "sensors", .flags = "v" },
#endif
#if FETCH_STATE
	{ .event = rcvOMKeyState, .seqid = rcvSeqsState, .lastSeq = 0, .state = SeqStateInit, .key = "state", .flags = "vn" },
#endif
#if FETCH_VOLUMES
	{ .event = rcvOMKeyVolumes, .seqid = rcvSeqsVolumes, .lastSeq = 0, .state = SeqStateInit, .key = "volumes", .flags = "v" },
#endif
};

static struct Seq *currentReqSeq = nullptr;
static struct Seq *currentRespSeq = nullptr;

static struct Seq* GetNextSeq(struct Seq *current)
{
	if (current == nullptr)
	{
		current = seqs;
	}


	for (size_t i = current - seqs; i < ARRAY_SIZE(seqs); ++i)
	{
		current = &seqs[i];
		if (current->state == SeqStateError)
		{
			// skip and re-init if last request had an error
			current->state = SeqStateInit;
			continue;
		}
		if (current->state == SeqStateInit || current->state == SeqStateUpdate)
		{
			return current;
		}
	}


	return nullptr;
}

static struct Seq *FindSeqByKey(const char *key)
{
	dbg("key %s", key);

	for (size_t i = 0; i < ARRAY_SIZE(seqs); ++i)
	{
		if (strcasecmp(seqs[i].key, key) == 0)
		{
			return &seqs[i];
		}

	}

	return nullptr;
}

static void UpdateSeq(const ReceivedDataEvent seqid, int32_t val)
{
	for (size_t i = 0; i < ARRAY_SIZE(seqs); ++i)
	{
		if (seqs[i].seqid == seqid)
		{
			dbg("%s %d %d", seqs[i].key, seqs[i].lastSeq, val);
			if (seqs[i].lastSeq != val)
			{
				seqs[i].lastSeq = val;
				seqs[i].state = SeqStateUpdate;
			}
		}
	}
}

static void ResetSeqs()
{
	for (size_t i = 0; i < ARRAY_SIZE(seqs); ++i)
	{
		seqs[i].lastSeq = 0;
		seqs[i].state = SeqStateInit;
	}
}

// Return the host firmware features
FirmwareFeatureMap GetFirmwareFeatures()
{
	return firmwareFeatures;
}

// Strip the drive letter prefix from a file path if the host firmware doesn't support it
const char* _ecv_array CondStripDrive(const char* _ecv_array arg)
{
	return ((firmwareFeatures.IsBitSet(noDriveNumber)) != 0 && isdigit(arg[0]) && arg[1] == ':')
			? arg + 2
			: arg;
}

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

bool IsPrintingStatus(OM::PrinterStatus status)
{
	return status == OM::PrinterStatus::printing
			|| status == OM::PrinterStatus::paused
			|| status == OM::PrinterStatus::pausing
			|| status == OM::PrinterStatus::resuming
			|| status == OM::PrinterStatus::simulating;
}

bool PrintInProgress()
{
	return IsPrintingStatus(status);
}

template<typename T>
int compare(const void* lp, const void* rp)
{
	return strcasecmp(((T*) lp)->key, ((T*) rp)->key);
}

// Return true if sending a command or file list request to the printer now is a good idea.
// We don't want to send these when the printer is busy with a previous command, because they will block normal status requests.
static bool OkToSend()
{
	return status == OM::PrinterStatus::idle
			|| status == OM::PrinterStatus::printing
			|| status == OM::PrinterStatus::paused
			|| status == OM::PrinterStatus::off;
}

// Return the printer status
OM::PrinterStatus GetStatus()
{
	return status;
}

// Initialise the LCD and user interface. The non-volatile data must be set up before calling this.
void InitLcd()
{
	lcd.InitLCD(nvData.lcdOrientation, IS_24BIT, IS_ER);				// set up the LCD
	colours = &colourSchemes[nvData.colourScheme];
	UI::InitColourScheme(colours);
	UI::CreateFields(nvData.language, *colours, nvData.infoTimeout);	// create all the fields
	lcd.fillScr(black);													// make sure the memory is clear
	Delay(100);															// give the LCD time to update
	RestoreBrightness();												// turn the display on
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

bool IsSaveNeeded()
{
	return nvData != savedNvData;
}

void MirrorDisplay()
{
	nvData.lcdOrientation = static_cast<DisplayOrientation>(nvData.lcdOrientation ^ ReverseX);
	lcd.setOrientation(nvData.lcdOrientation, IS_ER, true);
}

void InvertDisplay()
{
	nvData.lcdOrientation = static_cast<DisplayOrientation>(nvData.lcdOrientation ^ (ReverseX | ReverseY));
	lcd.setOrientation(nvData.lcdOrientation, IS_ER, true);
}

void LandscapeDisplay(const bool withTouch)
{
	lcd.fillScr(black);
	lcd.setOrientation(nvData.lcdOrientation, IS_ER, true);
	if (withTouch)
	{
		touch.init(DisplayX, DisplayY, nvData.touchOrientation);
		touch.calibrate(nvData.xmin, nvData.xmax, nvData.ymin, nvData.ymax, touchCalibMargin);
	}
}

void PortraitDisplay(const bool withTouch)
{
	DisplayOrientation portrait = static_cast<DisplayOrientation>(nvData.lcdOrientation ^ (SwapXY | ReverseX));
	lcd.fillScr(black);
	lcd.setOrientation(portrait, IS_ER, true);
	if (withTouch)
	{
		DisplayOrientation portraitTouch = static_cast<DisplayOrientation>(nvData.touchOrientation ^ (SwapXY | ReverseX));
		touch.init(DisplayXP, DisplayYP, portraitTouch);
		touch.calibrate(nvData.ymin, nvData.ymax, nvData.xmin, nvData.xmax, touchCalibMargin);
	}
}

void SetBaudRate(uint32_t rate)
{
	nvData.baudRate = rate;
	SerialIo::SetBaudRate(rate);
}

extern void SetBrightness(int percent)
{
	nvData.brightness = constrain<int>(percent, Buzzer::MinBrightness, Buzzer::MaxBrightness);
	RestoreBrightness();
}

void UpdatePollRate(bool idle)
{
	if (idle)
	{
		printerPollInterval = slowPrinterPollInterval;
	}
	else
	{
		printerPollInterval = defaultPrinterPollInterval * pollIntervalMultiplier;
		printerResponseInterval = defaultPrinterResponseInterval * pollIntervalMultiplier;
	}
}

void DeactivateScreensaver()
{
	if (screensaverActive) {
		UI::DeactivateScreensaver();
		screensaverActive = false;
		UpdatePollRate(screensaverActive);
	}
}

extern void RestoreBrightness()
{
	Buzzer::SetBacklight(nvData.brightness);
	lastActionTime = SystemTick::GetTickCount();
	isDimmed = false;
	DeactivateScreensaver();
}

extern void DimBrightness()
{
	if (   (nvData.displayDimmerType == DisplayDimmerType::always)
		|| (nvData.displayDimmerType == DisplayDimmerType::onIdle && (status == OM::PrinterStatus::idle || status == OM::PrinterStatus::off))
	   )
	{
		Buzzer::SetBacklight(nvData.brightness/8);
		isDimmed = true;
	}
}

void ActivateScreensaver()
{
	if (!screensaverActive)
	{
		if (!isDimmed)
		{
			Buzzer::SetBacklight(nvData.brightness/4);	// If the user disabled dimming we do it still here
			isDimmed = true;
		}
		screensaverActive = true;
		UI::ActivateScreensaver();
		UpdatePollRate(screensaverActive);
	}
	else
	{
		UI::AnimateScreensaver();
	}
}

DisplayDimmerType GetDisplayDimmerType()
{
	return nvData.displayDimmerType;
}

void SetDisplayDimmerType(DisplayDimmerType newType)
{
	nvData.displayDimmerType = newType;
}

void SetVolume(uint8_t newVolume)
{
	nvData.touchVolume = newVolume;
}

void SetInfoTimeout(uint8_t newInfoTimeout)
{
	nvData.infoTimeout = newInfoTimeout;
}

void SetScreensaverTimeout(uint32_t screensaverTimeout)
{
	nvData.screensaverTimeout = screensaverTimeout;
}

bool SetColourScheme(uint8_t newColours)
{
	const bool ret = (newColours != nvData.colourScheme);
	nvData.colourScheme = newColours;
	return ret;
}

// Set the language, returning true if it has changed
bool SetLanguage(uint8_t newLanguage)
{
	const bool ret = (newLanguage != nvData.language);
	nvData.language = newLanguage;
	return ret;
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

uint32_t GetScreensaverTimeout() {
	return nvData.screensaverTimeout;
}

uint8_t GetBabystepAmountIndex()
{
	return nvData.babystepAmountIndex;
}

void SetBabystepAmountIndex(uint8_t babystepAmountIndex)
{
	nvData.babystepAmountIndex = babystepAmountIndex;
}

uint16_t GetFeedrate()
{
	return nvData.feedrate;
}

void SetFeedrate(uint16_t feedrate)
{
	nvData.feedrate = feedrate;
}

HeaterCombineType GetHeaterCombineType()
{
	return nvData.heaterCombineType;
}

void SetHeaterCombineType(HeaterCombineType combine)
{
	nvData.heaterCombineType = combine;
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
	Reset();														// reset the processor
}

// Save settings
void SaveSettings()
{
	while (Buzzer::Noisy()) { }
	nvData.Save();
	// To make sure it worked, load the settings again
	savedNvData.Load();
}

// This is called when the status changes
static void SetStatus(OM::PrinterStatus newStatus)
{
	if (newStatus != status)
	{
		dbg("printer status %d -> %d", status, newStatus);
		if (isDimmed)
		{
			RestoreBrightness();
		}
		UI::ChangeStatus(status, newStatus);

		status = newStatus;
		UI::UpdatePrintingFields();
	}
}

// Set the status back to "Connecting"
static void Reconnect()
{
	dbg("Reconnect");

	initialized = false;
	lastPollTime = 0;
	lastResponseTime = 0;
	lastOutOfBufferResponse = 0;

	SetStatus(OM::PrinterStatus::connecting);
	ResetSeqs();

	UI::LastJobFileNameAvailable(false);
	UI::SetSimulatedTime(0);
	UI::UpdateDuration(0);
	UI::UpdateWarmupDuration(0);
}

// Try to get an integer value from a string. If it is actually a floating point value, round it.
bool GetInteger(const char s[], int32_t &rslt)
{
	if (s[0] == 0) return false;			// empty string

	const char* endptr;
	rslt = (int) StrToI32(s, &endptr);
	if (*endptr == 0) return true;			// we parsed an integer

	const float d = SafeStrtof(s, &endptr);		// try parsing a floating point number
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
	const char* endptr;
	rslt = (int) StrToU32(s, &endptr);
	return *endptr == 0;
}

// Try to get a floating point value from a string. if it is actually a floating point value, round it.
bool GetFloat(const char s[], float &rslt)
{
	if (s[0] == 0) return false;			// empty string
	const char* endptr;
	rslt = SafeStrtof(s, &endptr);
	return *endptr == 0;					// we parsed a float
}

// Try to get a bool value from a string.
bool GetBool(const char s[], bool &rslt)
{
	if (s[0] == 0) return false;			// empty string

	rslt = (strcasecmp(s, "true") == 0);
	return true;
}

static void StartReceivedMessage();
static void EndReceivedMessage();
static void ProcessReceivedValue(StringRef id, const char data[], const size_t indices[]);
static void ProcessArrayEnd(const char id[], const size_t indices[]);
static void ParserErrorEncountered(int currentState, const char*, const char*, const size_t[]);

static struct SerialIo::SerialIoCbs serial_cbs = {
	.StartReceivedMessage = StartReceivedMessage,
	.EndReceivedMessage = EndReceivedMessage,
	.ProcessReceivedValue = ProcessReceivedValue,
	.ProcessArrayEnd = ProcessArrayEnd,
	.ParserErrorEncountered = ParserErrorEncountered
};

static void StartReceivedMessage()
{
	dbg2();
	newMessageSeq = messageSeq;
	MessageLog::BeginNewMessage();
	FileManager::BeginNewMessage();
	currentAlert.flags.Clear();
	dbg2();
}

static void EndReceivedMessage()
{
	dbg2();

	lastResponseTime = SystemTick::GetTickCount();

	if (currentRespSeq != nullptr)
	{
		currentRespSeq->state = outOfBuffers ? SeqStateError : SeqStateOk;
		dbg("seq %s %d DONE", currentRespSeq->key, currentRespSeq->state);
		currentRespSeq = nullptr;
	}
	outOfBuffers = false;							// Reset the out-of-buffers flag

	if (newMessageSeq != messageSeq)
	{
		messageSeq = newMessageSeq;
		MessageLog::DisplayNewMessage();
	}
	FileManager::EndReceivedMessage();
	if (currentAlert.flags.IsBitSet(Alert::GotMode) && currentAlert.mode < 0)
	{
		UI::ClearAlert();
	}
	else if (currentAlert.AllFlagsSet() && currentAlert.seq != lastAlertSeq)
	{
		UI::ProcessAlert(currentAlert);
		lastAlertSeq = currentAlert.seq;
	}
	dbg2();
}

void HandleOutOfBufferResponse() {

	const uint32_t now = SystemTick::GetTickCount();

	// We received the previous out-of-buffer within 10s
	if (now - lastOutOfBufferResponse <= 10000) {
		++oobCounter;
	} else {
		// Reset it if it was more than 10s ago - glitches happen
		oobCounter = 0;
	}

	// Slow down the poll interval by 10% if we see too many out-of-buffer in short time
	if (oobCounter >= 3) {
		pollIntervalMultiplier += 0.1;
		UpdatePollRate(screensaverActive);
		oobCounter = 0;
		MessageLog::AppendMessage("Slowing down poll rate");
	}
	lastOutOfBufferResponse = now;
	outOfBuffers = true;
}

// Public functions called by the SerialIo module
static void ProcessReceivedValue(StringRef id, const char data[], const size_t indices[])
{
	ReceivedDataEvent currentResponseType = ReceivedDataEvent::rcvUnknown;

	if (StringStartsWith(id.c_str(), "result"))
	{
		// We might either get something like:
		// * "result[optional modified]:[key]:[field]" for a live response or
		// * "result[optional modified]:[field]" for a detailed response
		// If live response remove "result:"
		// else replace "result" by "key" (do NOT replace anything beyond "result" as there might be an _ecv_array modifier)

		id.Erase(0, 6);
		if (currentRespSeq != nullptr)
		{
			currentResponseType = currentRespSeq->seqid;
			id.Prepend(currentRespSeq->key);
		}
		else
		{
			// if empty key also erase the colon
			id.Erase(0);
		}
	}

	const FieldTableEntry key = { ReceivedDataEvent::rcvUnknown, id.c_str()};
	const FieldTableEntry* searchResult = (FieldTableEntry*) bsearch(
			&key,
			fieldTable,
			ARRAY_SIZE(fieldTable),
			sizeof(FieldTableEntry),
			compare<FieldTableEntry>);

	// no matching key found
	if (!searchResult)
	{
		dbg("no matching key found for %s", id.c_str());
		return;
	}
	const ReceivedDataEvent rde = searchResult->val;
	//dbg("event: %s(%d) rtype %d data '%s'", searchResult->key, searchResult->val, currentResponseType, data);
	switch (rde)
	{
	// M409 section
	case rcvKey:
		dbg2();
		{
			// try a quick check otherwise search for key
			if (currentReqSeq && (strcasecmp(data, currentReqSeq->key) == 0))
			{
				currentRespSeq = currentReqSeq;
			}
			else
			{
				currentRespSeq = FindSeqByKey(data);
			}

			if (currentRespSeq == nullptr)
			{
				break;
			}

			// reset processing variables
			switch (currentRespSeq->event) {
			case rcvOMKeyHeat:
				lastBed = -1;
				lastChamber = -1;
				break;
			case rcvOMKeyMove:
				visibleAxesCounted = 0;
				break;
			case rcvOMKeySpindles:
				lastSpindle = -1;
				break;
			case rcvOMKeyTools:
				lastTool = -1;
				break;
			case rcvOMKeyVolumes:
				mountedVolumesCounted = 0;
				break;
			default:
				break;
			}
		}
		break;

	// Boards section
	case rcvBoardsFirmwareName:
		dbg2();
		if (indices[0] == 0)			// currently we only handle the first board
		{
			for (size_t i = 0; i < ARRAY_SIZE(firmwareTypes); ++i)
			{
				if (StringStartsWith(data, firmwareTypes[i].name))
				{
					const FirmwareFeatureMap newFeatures = firmwareTypes[i].features;
					if (newFeatures != firmwareFeatures)
					{
						firmwareFeatures = newFeatures;
						UI::FirmwareFeaturesChanged(firmwareFeatures);
						FileManager::FirmwareFeaturesChanged();
					}
					break;
				}
			}
		}
		break;

	// Fans section
	case rcvFansRequestedValue:
		dbg2();
		{
			float f;
			bool b = GetFloat(data, f);
			if (b && f >= 0.0 && f <= 1.0)
			{
				UI::UpdateFanPercent(indices[0], (int)((f * 100.0f) + 0.5f));
			}
		}
		break;

	// Heat section
	case rcvHeatBedHeaters:
		dbg2();
		{
			int32_t heaterNumber;
			if (GetInteger(data, heaterNumber) && heaterNumber > -1)
			{
				UI::SetBedOrChamberHeater(indices[0], heaterNumber);
				for (size_t i = lastBed + 1; i < indices[0]; ++i)
				{
					OM::RemoveBed(i, false);
				}
				lastBed = indices[0];
			}
		}
		break;

	case rcvHeatChamberHeaters:
		dbg2();
		{
			int32_t heaterNumber;
			if (GetInteger(data, heaterNumber) && heaterNumber > -1)
			{
				UI::SetBedOrChamberHeater(indices[0], heaterNumber, false);
				for (size_t i = lastChamber + 1; i < indices[0]; ++i)
				{
					OM::RemoveChamber(i, false);
				}
				lastChamber = indices[0];
			}
		}
		break;

	case rcvHeatHeatersActive:
		dbg2();
		{
			int32_t ival;
			if (GetInteger(data, ival))
			{
				UI::UpdateActiveTemperature(indices[0], ival);
			}
		}
		break;

	case rcvHeatHeatersCurrent:
		dbg2();
		{
			float fval;
			if (GetFloat(data, fval))
			{
				dbg2();
				UI::UpdateCurrentTemperature(indices[0], fval);
			}
		}
		break;

	case rcvHeatHeatersStandby:
		dbg2();
		{
			int32_t ival;
			if (GetInteger(data, ival))
			{
				UI::UpdateStandbyTemperature(indices[0], ival);
			}
		}
		break;

	case rcvHeatHeatersState:
		dbg2();
		{
			const OM::HeaterStatusMapEntry key = (OM::HeaterStatusMapEntry) {data, OM::HeaterStatus::off};
			const OM::HeaterStatusMapEntry * statusFromMap =
					(OM::HeaterStatusMapEntry *) bsearch(
							&key,
							OM::heaterStatusMap,
							ARRAY_SIZE(OM::heaterStatusMap),
							sizeof(OM::HeaterStatusMapEntry),
							compare<OM::HeaterStatusMapEntry>);
			const OM::HeaterStatus status = (statusFromMap != nullptr) ? statusFromMap->val : OM::HeaterStatus::off;
			UI::UpdateHeaterStatus(indices[0], status);
		}
		break;

	// Job section
	case rcvJobDuration:
		dbg2();
		{
			uint32_t duration;
			if (GetUnsignedInteger(data, duration))
			{
				UI::UpdateDuration(duration);
			}
			else
			{
				UI::UpdateDuration(0);
			}
		}
		break;

	case rcvJobFileFilename:
		dbg2();
		UI::PrintingFilenameChanged(data);
		break;

	case rcvJobFileSize:
		dbg2();
		{
			uint32_t ival;
			if (GetUnsignedInteger(data, ival))
			{
				fileSize = ival;
			}
			else
			{
				fileSize = 0;
			}
		}
		break;

	case rcvJobFileSimulatedTime:
		dbg2();
		{
			uint32_t simulatedTime;
			if (GetUnsignedInteger(data, simulatedTime))
			{
				UI::SetSimulatedTime(simulatedTime);
			}
			else
			{
				UI::SetSimulatedTime(0);
			}
		}
		break;

	case rcvJobFilePosition:
		dbg2();
		{
			if (PrintInProgress() && fileSize > 0)
			{
				uint32_t ival;
				if (GetUnsignedInteger(data, ival))
				{
					UI::SetPrintProgressPercent((unsigned int)(((ival*100.0f)/fileSize) + 0.5));
				}

			}
		}
		break;

	case rcvJobLastFileName:
		dbg2();
		UI::LastJobFileNameAvailable(true);	// If we get here there is a filename
		break;

	case rcvJobLastFileSimulated:
		dbg2();
		{
			bool lastFileSimulated;
			if (GetBool(data, lastFileSimulated))
			{
				UI::SetLastFileSimulated(lastFileSimulated);
			}
		}
		break;

	case rcvJobTimesLeftFile:
	case rcvJobTimesLeftFilament:
	case rcvJobTimesLeftSlicer:
		dbg2();
		{
			int32_t timeLeft;
			bool b = GetInteger(data, timeLeft);
			if (b && timeLeft >= 0 && timeLeft < 10 * 24 * 60 * 60 && PrintInProgress())
			{
				UI::UpdateTimesLeft((rde == rcvJobTimesLeftFilament) ? 1 : (rde == rcvJobTimesLeftSlicer) ? 2 : 0, timeLeft);
			}
		}
		break;

	case rcvJobWarmUpDuration:
		dbg2();
		{
			uint32_t warmUpDuration;
			if (GetUnsignedInteger(data, warmUpDuration))
			{
				UI::UpdateWarmupDuration(warmUpDuration);
			}
			else
			{
				UI::UpdateWarmupDuration(0);
			}
		}
		break;

	// Move section
	case rcvMoveAxesBabystep:
		dbg2();
		{
			float f;
			if (GetFloat(data, f))
			{
				UI::SetBabystepOffset(indices[0], f);
			}
		}
		break;

	case rcvMoveAxesHomed:
		dbg2();
		{
			bool isHomed;
			if (indices[0] < MaxTotalAxes && GetBool(data, isHomed))
			{
				UI::UpdateHomedStatus(indices[0], isHomed);
			}
		}
		break;

	case rcvMoveAxesLetter:
		dbg2();
		{
			UI::SetAxisLetter(indices[0], data[0]);
		}
		break;

	case rcvMoveAxesUserPosition:
		dbg2();
		{
			float fval;
			if (GetFloat(data, fval))
			{
				UI::UpdateAxisPosition(indices[0], fval);
			}
		}
		break;

	case rcvMoveAxesVisible:
		dbg2();
		{
			bool visible;
			if (GetBool(data, visible))
			{
				UI::SetAxisVisible(indices[0], visible);
				if (visible)
				{
					++visibleAxesCounted;
				}
			}

		}
		break;

	case rcvMoveAxesWorkplaceOffsets:
		dbg2();
		{
			float offset;
			if (GetFloat(data, offset))
			{
				UI::SetAxisWorkplaceOffset(indices[0], indices[1], offset);
			}
		}
		break;

	case rcvMoveExtrudersFactor:
		dbg2();
		{
			float fval;
			if (GetFloat(data, fval))
			{
				UI::UpdateExtrusionFactor(indices[0], (int)((fval * 100.0f) + 0.5));
			}
		}
		break;

	case rcvMoveKinematicsName:
		dbg2();
		if (status != OM::PrinterStatus::configuring && status != OM::PrinterStatus::connecting)
		{
			isDelta = (strcasecmp(data, "delta") == 0);
			UI::UpdateGeometry(numAxes, isDelta);
		}
		break;

	case rcvMoveSpeedFactor:
		dbg2();
		{
			float fval;
			if (GetFloat(data, fval))
			{
				UI::UpdateSpeedPercent((int) ((fval * 100.0f) + 0.5f));
			}
		}
		break;

	case rcvMoveWorkplaceNumber:
		{
			uint32_t workplaceNumber;
			if (GetUnsignedInteger(data, workplaceNumber))
			{
				UI::SetCurrentWorkplaceNumber(workplaceNumber);
			}
		}
		break;

	// Network section
	case rcvNetworkName:
		dbg2();
		if (status != OM::PrinterStatus::configuring && status != OM::PrinterStatus::connecting)
		{
			UI::UpdateMachineName(data);
		}
		break;

	case rcvNetworkInterfacesActualIP:
		{
			// Only look at the first valid IP
			if (indices[0] > 0)
			{
				return;
			}
			UI::UpdateIP(data);
		}
		break;

	// Seqs section
	case rcvSeqsBoards:
	case rcvSeqsDirectories:
	case rcvSeqsFans:
	case rcvSeqsHeat:
	case rcvSeqsInputs:
	case rcvSeqsJob:
	case rcvSeqsMove:
	case rcvSeqsNetwork:
	case rcvSeqsReply:
	case rcvSeqsScanner:
	case rcvSeqsSensors:
	case rcvSeqsSpindles:
	case rcvSeqsState:
	case rcvSeqsTools:
	case rcvSeqsVolumes:
		dbg2();
		{
			int32_t ival;

			if (GetInteger(data, ival))
			{
				UpdateSeq(rde, ival);
			}

		}
		break;

	// Sensors section
	case rcvSensorsProbeValue:
		dbg2();
		{
			if (indices[0] == 0 && indices[1] == 0)			// currently we only handle one probe with one value
			UI::UpdateZProbe(data);
		}
		break;

	// Spindles section
	case rcvSpindlesActive:
		dbg2();
		{
			int32_t active;
			if (GetInteger(data, active))
			{
				if (active < 0)
				{
					firmwareFeatures.SetBit(m568TempAndRPM);
				}
				UI::SetSpindleActive(indices[0], active);
			}

			for (size_t i = lastSpindle + 1; i < indices[0]; ++i)
			{
				OM::RemoveSpindle(i, false);
			}
			lastSpindle = indices[0];
		}
		break;

	case rcvSpindlesCurrent:
		dbg2();
		{
			int32_t current;
			if (GetInteger(data, current))
			{
				if (current < 0)
				{
					firmwareFeatures.SetBit(m568TempAndRPM);
				}
				UI::SetSpindleCurrent(indices[0], current);
			}
		}
		break;

	case rcvSpindlesMax:
	case rcvSpindlesMin:
		dbg2();
		// fans also has a field "result^:max"
		if (currentResponseType != rcvOMKeySpindles)
		{
			break;
		}
		{
			uint32_t speedLimit;
			if (GetUnsignedInteger(data, speedLimit))
			{
				UI::SetSpindleLimit(indices[0], speedLimit, rde == rcvSpindlesMax);
			}
		}
		break;

	case rcvSpindlesState:
		dbg2();
		{
			const OM::SpindleStateMapEntry key = (OM::SpindleStateMapEntry) {data, OM::SpindleState::stopped};
			const OM::SpindleStateMapEntry * statusFromMap =
					(OM::SpindleStateMapEntry *) bsearch(
							&key,
							OM::spindleStateMap,
							ARRAY_SIZE(OM::spindleStateMap),
							sizeof(OM::SpindleStateMapEntry),
							compare<OM::SpindleStateMapEntry>);
			const OM::SpindleState state = (statusFromMap != nullptr) ? statusFromMap->val : OM::SpindleState::stopped;
			UI::SetSpindleState(indices[0], state);
		}
		break;

	case rcvSpindlesTool:
		dbg2();
		{
			int32_t toolNumber;
			if (GetInteger(data, toolNumber))
			{
				firmwareFeatures.ClearBit(m568TempAndRPM);
				UI::SetSpindleTool(indices[0], toolNumber);
			}
		}
		break;

	// State section
	case rcvStateCurrentTool:
		dbg2();
		if (status == OM::PrinterStatus::connecting)
		{
			break;
		}
		{
			int32_t tool;
			if (GetInteger(data, tool))
			{
				UI::SetCurrentTool(tool);
			}
		}
		break;

	case rcvStateMessageBox:
		dbg2();
		// Nessage box has been dealt with somewhere else
		if (data[0] == 0)
		{
			UI::ClearAlert();
		}
		break;

	case rcvStateMessageBoxAxisControls:
		dbg2();
		if (GetUnsignedInteger(data, currentAlert.controls))
		{
			currentAlert.flags.SetBit(Alert::GotControls);
		}
		break;

	case rcvStateMessageBoxMessage:
		dbg2();
		currentAlert.text.copy(data);
		currentAlert.flags.SetBit(Alert::GotText);
		break;

	case rcvStateMessageBoxMode:
		dbg2();
		if (GetInteger(data, currentAlert.mode))
		{
			currentAlert.flags.SetBit(Alert::GotMode);
		}
		break;

	case rcvStateMessageBoxSeq:
		dbg2();
		if (GetUnsignedInteger(data, currentAlert.seq))
		{
			currentAlert.flags.SetBit(Alert::GotSeq);
		}
		break;

	case rcvStateMessageBoxTimeout:
		dbg2();
		if (GetFloat(data, currentAlert.timeout))
		{
			currentAlert.flags.SetBit(Alert::GotTimeout);
		}
		break;

	case rcvStateMessageBoxTitle:
		dbg2();
		currentAlert.title.copy(data);
		currentAlert.flags.SetBit(Alert::GotTitle);
		break;

	case rcvStateStatus:
		dbg2();
		{
			const OM::PrinterStatusMapEntry key = (OM::PrinterStatusMapEntry) { .key = data, .val = OM::PrinterStatus::connecting};
			const OM::PrinterStatusMapEntry * statusFromMap =
					(OM::PrinterStatusMapEntry *) bsearch(
							&key,
							OM::printerStatusMap,
							ARRAY_SIZE(OM::printerStatusMap),
							sizeof(OM::PrinterStatusMapEntry),
							compare<OM::PrinterStatusMapEntry>);
			if (!statusFromMap)
			{
				dbg("unknown status %s", data);
				break;
			}
			SetStatus(statusFromMap->val);
		}
		break;

	case rcvStateUptime:
		dbg2();
		{
			uint32_t uival;
			if (GetUnsignedInteger(data, uival))
			{
				// Controller was restarted
				if (uival < remoteUpTime)
				{
					Reconnect();
				}
				remoteUpTime = uival;
			}
		}
		break;

	// Tools section
	case rcvToolsActive:
	case rcvToolsStandby:
		dbg2();
		{
			if (indices[1] >= MaxHeatersPerTool)
			{
				break;
			}
			int32_t temp;
			if (GetInteger(data, temp))
			{
				UI::UpdateToolTemp(indices[0], indices[1], temp, rde == rcvToolsActive);
			}
		}
		break;

	case rcvToolsExtruders:
		dbg2();
		{
			uint32_t extruder;
			if (GetUnsignedInteger(data, extruder))
			{
				UI::SetToolExtruder(indices[0], extruder);
			}
		}
		break;

	case rcvToolsFans:
		dbg2();
		{
			uint32_t fan;
			if (GetUnsignedInteger(data, fan))
			{
				UI::SetToolFan(indices[0], fan);
			}
		}
		break;

	case rcvToolsHeaters:
		dbg2();
		{
			if (indices[1] >= MaxHeatersPerTool)
			{
				break;
			}
			uint32_t heaterIndex;
			if (GetUnsignedInteger(data, heaterIndex))
			{
				UI::SetToolHeater(indices[0], indices[1], heaterIndex);
			}
		}
		break;

	case rcvToolsNumber:
		dbg2();
		{
			for (size_t i = lastTool + 1; i < indices[0]; ++i)
			{
				OM::RemoveTool(i, false);
			}
			lastTool = indices[0];
		}
		break;

	case rcvToolsOffsets:
		dbg2();
		{
			float offset;
			if (GetFloat(data, offset))
			{
				UI::SetToolOffset(indices[0], indices[1], offset);
			}
		}
		break;

	case rcvToolsSpindle:
		dbg2();
		{
			int32_t spindleNumber;
			if (GetInteger(data, spindleNumber))
			{
				firmwareFeatures.SetBit(m568TempAndRPM);
				UI::SetToolSpindle(indices[0], spindleNumber);
			}
		}
		break;

	case rcvToolsState:
		dbg2();
		{
			const OM::ToolStatusMapEntry key = (OM::ToolStatusMapEntry) {data, OM::ToolStatus::off};
			const OM::ToolStatusMapEntry * statusFromMap =
					(OM::ToolStatusMapEntry *) bsearch(
							&key,
							OM::toolStatusMap,
							ARRAY_SIZE(OM::toolStatusMap),
							sizeof(OM::ToolStatusMapEntry),
							compare<OM::ToolStatusMapEntry>);
			const OM::ToolStatus status = (statusFromMap != nullptr) ? statusFromMap->val : OM::ToolStatus::off;
			UI::UpdateToolStatus(indices[0], status);
		}
		break;

	// Volumes section
	case rcvVolumesMounted:
		dbg2();
		{
			bool mounted;
			if (GetBool(data, mounted) && mounted)
			{
				++mountedVolumesCounted;
			}
		}
		break;

	// Push messages
	case rcvPushResponse:
		dbg2();
		MessageLog::SaveMessage(data);
		break;

	case rcvPushMessage:
		dbg2();
		if (data[0] == 0)
		{
			UI::ClearAlert();
		}
		else
		{
			UI::ProcessSimpleAlert(data);
		}
		break;

	case rcvPushSeq:
		dbg2();
		GetUnsignedInteger(data, newMessageSeq);
		break;

	case rcvPushBeepDuration:
		dbg2();
		GetInteger(data, beepLength);
		break;

	case rcvPushBeepFrequency:
		dbg2();
		GetInteger(data, beepFrequency);
		break;

	// M20 section
	case rcvM20Dir:
		dbg2();
		FileManager::ReceiveDirectoryName(data);
		break;

	case rcvM20Err:
		dbg2();
		{
			int32_t i;
			if (GetInteger(data, i))
			{
				if (i >= 0)
				{
					FileManager::ReceiveErrorCode(i);
				}
				else if (i == -1)
				{
					// This is not actually part of M20 but RRF ran out of buffers
					HandleOutOfBufferResponse();
				}
			}
		}
		break;

	case rcvM20Files:
		dbg2();
		if (indices[0] == 0)
		{
			FileManager::BeginReceivingFiles();
		}
		FileManager::ReceiveFile(data);
		break;

	// M36 section
	case rcvM36Filament:
		dbg2();
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
	case rcvM36Filename:
		break;

	case rcvM36GeneratedBy:
		dbg2();
		UI::UpdateFileGeneratedByText(data);
		break;

	case rcvM36Height:
		dbg2();
		{
			float f;
			if (GetFloat(data, f))
			{
				UI::UpdateFileObjectHeight(f);
			}
		}
		break;

	case rcvM36LastModified:
		dbg2();
		UI::UpdateFileLastModifiedText(data);
		break;

	case rcvM36LayerHeight:
		dbg2();
		{
			float f;
			if (GetFloat(data, f))
			{
				UI::UpdateFileLayerHeight(f);
			}
		}
		break;

	case rcvM36PrintTime:
	case rcvM36SimulatedTime:
		dbg2();
		{
			int32_t sz;
			if (GetInteger(data, sz) && sz > 0)
			{
				UI::UpdatePrintTimeText((uint32_t)sz, rde == rcvM36SimulatedTime);
			}
		}
		break;

	case rcvM36Size:
		dbg2();
		{
			int32_t sz;
			if (GetInteger(data, sz))
			{
				UI::UpdateFileSize(sz);
			}
		}
		break;

	case rcvControlCommand:
		dbg2();
		{
			const ControlCommandMapEntry key = (ControlCommandMapEntry) {data, ControlCommand::invalid};
			const ControlCommandMapEntry * controlCommandFromMap =
					(ControlCommandMapEntry *) bsearch(
							&key,
							controlCommandMap,
							ARRAY_SIZE(controlCommandMap),
							sizeof(ControlCommandMapEntry),
							compare<ControlCommandMapEntry>);
			const ControlCommand controlCommand = (controlCommandFromMap != nullptr) ? controlCommandFromMap->val : ControlCommand::invalid;
			switch (controlCommand)
			{
			case ControlCommand::eraseAndReset:
				EraseAndReset();					// Does not return
				break;
			case ControlCommand::reset:
				Reset();							// Does not return
				break;
			default:
				// Invalid command. Just ignore.
				break;
			}
		}
		break;

	default:
		break;
	}
	dbg2();
}

// Public function called when the serial I/O module finishes receiving an array of values
static void ProcessArrayEnd(const char id[], const size_t indices[])
{
	ReceivedDataEvent currentResponseType = currentRespSeq != nullptr ? currentRespSeq->event : ReceivedDataEvent::rcvUnknown;
	if (indices[0] == 0 && strcmp(id, "files^") == 0)
	{
		dbg2();
		FileManager::BeginReceivingFiles();				// received an empty file list - need to tell the file manager about it
	}
	else if (currentResponseType == rcvOMKeyHeat)
	{
		if (strcasecmp(id, "heat:bedHeaters^") == 0)
		{
			dbg2();
			OM::RemoveBed(lastBed + 1, true);
			if (initialized)
			{
				UI::AllToolsSeen();
			}
		}
		else if (strcasecmp(id, "heat:chamberHeaters^") == 0)
		{
			dbg2();
			OM::RemoveChamber(lastChamber + 1, true);
			if (initialized)
			{
				UI::AllToolsSeen();
			}
		}
	}
	else if (currentResponseType == rcvOMKeyMove && strcasecmp(id, "move:axes^") == 0)
	{
		dbg2();
		OM::RemoveAxis(indices[0], true);
		numAxes = constrain<unsigned int>(visibleAxesCounted, MIN_AXES, MaxDisplayableAxes);
		UI::UpdateGeometry(numAxes, isDelta);
	}
	else if (currentResponseType == rcvOMKeySpindles)
	{
		if (strcasecmp(id, "spindles^") == 0)
		{
			dbg2();
			OM::RemoveSpindle(lastSpindle + 1, true);
			if (initialized)
			{
				UI::AllToolsSeen();
			}
		}
	}
	else if (currentResponseType == rcvOMKeyTools)
	{
		if (strcasecmp(id, "tools^") == 0)
		{
			dbg2();
			OM::RemoveTool(lastTool + 1, true);
			if (initialized)
			{
				UI::AllToolsSeen();
			}
		}
		else if (strcasecmp(id, "tools^:extruders^") == 0 && indices[1] == 0)
		{
			dbg2();
			UI::SetToolExtruder(indices[0], -1);			// No extruder defined for this tool
		}
		else if (strcasecmp(id, "tools^:heaters^") == 0)
		{
			dbg2();
			// Remove all heaters no longer defined
			if (UI::RemoveToolHeaters(indices[0], indices[1]) && initialized)
			{
				UI::AllToolsSeen();
			}
		}
	}
	else if (currentResponseType == rcvOMKeyVolumes && strcasecmp(id, "volumes^") == 0)
	{
		dbg2();
		FileManager::SetNumVolumes(mountedVolumesCounted);
	}
}

static void ParserErrorEncountered(int currentState, const char *id, const char*data, const size_t arraysize[])
{
	MessageLog::AppendMessageF("Error parsing response %s in state %d", id, currentState);
	if (currentRespSeq == nullptr)
	{
		return;
	}

	currentRespSeq->state = SeqStateError;
}

// Update those fields that display debug information
void UpdateDebugInfo()
{
	freeMem->SetValue(GetFreeMemory());
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
	InitMemory();

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
		InitLcd();
		touch.init(DisplayX, DisplayY, nvData.touchOrientation);
		touch.calibrate(nvData.xmin, nvData.xmax, nvData.ymin, nvData.ymax, touchCalibMargin);
		savedNvData = nvData;
	}
	else
	{
		// The touch panel has not been calibrated, and we do not know which way up it is
		nvData.SetDefaults();
		InitLcd();
		CalibrateTouch();							// this includes the touch driver initialisation
		SaveSettings();
	}

	// Set up the baud rate
	SerialIo::Init(nvData.baudRate, &serial_cbs);

	MessageLog::Init();

	// Display the splash screen if one has been appended to the file, unless it was a software reset (we use software reset to change the language or colour scheme)
	// The splash screen data comprises the number of X pixels, then the number of Y pixels, then the data
	if (rstc_get_reset_cause(RSTC) != RSTC_SOFTWARE_RESET && _esplash[0] == DISPLAY_X && _esplash[1] == DISPLAY_Y)
	{
		lcd.fillScr(black);
		lcd.drawCompressedBitmapBottomToTop(0, 0, DISPLAY_X, DISPLAY_Y, _esplash + 2);
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

	mgr.Refresh(true);								// draw the screen for the first time
	UI::UpdatePrintingFields();

	// Hide all tools and heater related columns initially
	UI::AllToolsSeen();

	debugField->Show(false);					// show the debug field only if debugging is enabled

	// Display the Control tab. This also refreshes the display.
	UI::ShowDefaultPage();

	// Sort the fieldTable
	qsort(
			fieldTable,
			ARRAY_SIZE(fieldTable),
			sizeof(FieldTableEntry),
			compare<FieldTableEntry>);

	lastActionTime = SystemTick::GetTickCount();

	dbg("basic init DONE");

	for (;;)
	{
		dbg2();

		// 1. Check for input from the serial port and process it.
		// This calls back into functions StartReceivedMessage, ProcessReceivedValue, EndReceivedMessage and ParserErrorEncountered
		SerialIo::CheckInput();
		dbg2();

		// 2. if displaying the message log, update the times
		UI::Spin();
		dbg2();

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
				if (isDimmed || screensaverActive)
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
						if (!initialized)		// Last button press was E-Stop
						{
							continue;
						}
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
			else if (SystemTick::GetTickCount() - lastActionTime >= DimDisplayTimeout)
			{
				if (!isDimmed && UI::CanDimDisplay()){
					DimBrightness();				// it might not actually dim the display, depending on various flags
				}
				uint32_t screensaverTimeout = GetScreensaverTimeout();
				if (screensaverTimeout > 0 && SystemTick::GetTickCount() - lastActionTime >= screensaverTimeout)
				{
					ActivateScreensaver();
				}
			}
		}
		dbg2();

		// 4. Refresh the display
		UpdateDebugInfo();
		mgr.Refresh(false);
		dbg2();

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
		dbg2();

		// 6. If it is time, poll the printer status.
		// When the printer is executing a homing move or other file macro, it may stop responding to polling requests.
		// Under these conditions, we slow down the rate of polling to avoid building up a large queue of them.
		const uint32_t now = SystemTick::GetTickCount();
		if ((UI::DoPolling()										// don't poll while we are in the Setup page
		     && now - lastPollTime >= printerPollInterval			// if we haven't polled the printer too recently...
		     && now - lastResponseTime >= printerResponseInterval)	// and we haven't had a response too recently
		     || (!initialized && (now - lastPollTime > now - lastResponseTime))	// but if we are initializing do it as fast as possible where
		   )
		{
			if (now - lastPollTime > now - lastResponseTime)		// if we've had a response since the last poll
			{
				currentReqSeq = GetNextSeq(currentReqSeq);
				if (currentReqSeq != nullptr)
				{
					dbg("sending %s", currentReqSeq->key);
					SerialIo::Sendf("M409 K\"%s\" F\"%s\"\n", currentReqSeq->key, currentReqSeq->flags);
					lastPollTime = SystemTick::GetTickCount();
				}
				else
				{
					// Once we get here the first time we will have work all seqs once
					if (!initialized)
					{
						dbg("seqs init DONE");
						UI::AllToolsSeen();
						initialized = true;
					}

					// First check for specific info we need to fetch
					if (OkToSend())
					{
						bool sent = FileManager::ProcessTimers();
						// Otherwise just send a normal poll command
						if (!sent)
						{
							SerialIo::Sendf("M409 F\"d99f\"\n");
						}
						lastPollTime = SystemTick::GetTickCount();
					}

				}
			}
			else if (now - lastPollTime >= printerPollTimeout)	  // last response was most likely incomplete start over
			{
				SerialIo::Sendf("M409 F\"d99f\"\n");
				lastPollTime = SystemTick::GetTickCount();
			}
		}
		dbg2();
	}
}

void PrintDebugText(const char *x)
{
	fwVersionField->SetValue(x);
}

// Pure virtual function call handler, to avoid pulling in large chunks of the standard library
extern "C" void __cxa_pure_virtual() { while (1); }

// End
