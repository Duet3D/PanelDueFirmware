/*
 * Events.hpp
 *
 *  Created on: 6 Jan 2017
 *      Author: David
 */

#ifndef SRC_UI_EVENTS_HPP_
#define SRC_UI_EVENTS_HPP_

// Event numbers, used to say what we need to do when a field is touched
// *** MUST leave value 0 free to mean "no event"
enum Event : uint8_t
{
	evNull = 0,						// value must match nullEvent declared in Display.hpp

	evLandscape,
	evPortrait,

	// Page selection
	evTabControl, evTabStatus, evTabMsg, evTabSetup,

	// Pendant-mode page selection
	evTabJog, evTabOffset, evTabJob,

	// Pendant jog buttons
	evPJogAxis, evPJogAmount,

	// Pendant tool selection
	evToolSelect,

	evMeasureZ,

	// Pendant offset related
	evProbeWorkpiece, evTouchoff, evSetToolOffset, evFindCenterOfCavity, evZeroAxisInWCS, evWCSOffsetsPopup, evSetAxesOffsetToCurrent,
	evSelectAxisForWCSFineControl, evSelectAmountForWCSFineControl,

	// Heater control
	evSelectHead, evSelectBed, evSelectChamber,
	evAdjustToolActiveTemp, evAdjustToolStandbyTemp,
	evAdjustBedActiveTemp, evAdjustBedStandbyTemp,
	evAdjustChamberActiveTemp, evAdjustChamberStandbyTemp,

	// Spindle control
	evAdjustActiveRPM,

	// Control functions
	evMovePopup, evExtrudePopup, evFan, evListMacros,
	evMoveAxis,
	evMoveSelectAxis,
	evExtrudeAmount, evExtrudeRate, evExtrude, evRetract,
	evHomeAxis,
	
	evMoveAxisP,
	evExtrudeAmountP, evExtrudeRateP,
	evWCSDisplaySelect, evActivateWCS,

	// Print functions
	evExtrusionFactor,
	evAdjustFan,
	evAdjustInt,
	evSetInt,
	evListFiles,

	evFile, evMacro, evMacroControlPage,
	evPrintFile,
	evSendCommand,
	evFactoryReset,
	evAdjustSpeed,
	evPAdjustExtrusionPercent, // TODO: remove as soon as we have extruder number

	evScrollFiles, evScrollMacros, evFilesUp, evMacrosUp, evChangeCard,

	evKeyboard,

	// Setup functions
	evCalTouch, evSetBaudRate, evInvertX, evInvertY, evAdjustBaudRate, evSetVolume, evAdjustVolume, evSetInfoTimeout, evAdjustInfoTimeout, evReset,

	evYes,
	evCancel,
	evDeleteFile,
	evSimulateFile,
	evPausePrint,
	evResumePrint,
	evReprint, evResimulate,
	evBabyStepPopup, evBabyStepMinus, evBabyStepPlus,

	evKey, evShift, evBackspace, evSendKeyboardCommand, evUp, evDown,

	evAdjustLanguage, evSetLanguage,
	evAdjustColours, evSetColours,
	evBrighter, evDimmer,
	evSetDimmingType,
	evSetScreensaverTimeout, evAdjustScreensaverTimeout,
	evSetBabystepAmount, evAdjustBabystepAmount,
	evSetFeedrate, evAdjustFeedrate,
	evSetHeaterCombineType,

	evEmergencyStop,

	evJogZ, evCloseAlert
};

#endif /* SRC_UI_EVENTS_HPP_ */
