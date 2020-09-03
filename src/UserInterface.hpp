/*
 * UserInterface.hpp
 *
 *  Created on: 7 Jan 2017
 *      Author: David
 */

#ifndef SRC_USERINTERFACE_HPP_
#define SRC_USERINTERFACE_HPP_

#include "ColourSchemes.hpp"
#include "PrinterStatus.hpp"
#include "Display.hpp"
#include "FirmwareFeatures.hpp"
#include "Events.hpp"
#include "HeaterStatus.hpp"
#include "ToolStatus.hpp"

extern IntegerField *freeMem;
extern StaticTextField *debugField;
extern StaticTextField *touchCalibInstruction;
extern StaticTextField *messageTextFields[], *messageTimeFields[];
extern TextField *fwVersionField;

class Alert;

namespace UI
{
	extern unsigned int GetNumLanguages();
	extern void CreateFields(uint32_t language, const ColourScheme& colours, uint32_t p_infoTimeout);
	extern void ShowAxis(size_t axis, bool b);
	extern void UpdateAxisPosition(size_t axis, float fval);
	extern void UpdateCurrentTemperature(size_t heater, float fval);
	extern void UpdateHeaterStatus(const size_t heater, const HeaterStatus status);
	extern void ChangeStatus(PrinterStatus oldStatus, PrinterStatus newStatus);
	extern void UpdateTimesLeft(size_t index, unsigned int seconds);
	extern bool ChangePage(ButtonBase *newTab);
	extern bool DoPolling();
	extern void Tick();
	extern void Spin();
	extern void PrintStarted();
	extern void PrintingFilenameChanged(const char data[]);
	extern void ShowDefaultPage();
	extern void UpdatePrintingFields();
	extern void SetPrintProgressPercent(unsigned int percent);
	extern void UpdateGeometry(unsigned int p_numAxes, bool p_isDelta);
	extern void UpdateHomedStatus(size_t axis, bool isHomed);
	extern void UpdateZProbe(const char data[]);
	extern void UpdateMachineName(const char data[]);
	extern void ProcessAlert(const Alert& alert);
	extern void ClearAlert();
	extern void ProcessSimpleAlert(const char* array text);
	extern void NewResponseReceived(const char* array text);
	extern bool CanDimDisplay();
	extern void UpdateFileLastModifiedText(const char data[]);
	extern void UpdateFileGeneratedByText(const char data[]);
	extern void UpdateFileObjectHeight(float f);
	extern void UpdateFileLayerHeight(float f);
	extern void UpdateFileSize(int size);
	extern void UpdateFileFilament(int len);
	extern void UpdateFanPercent(int rpm);
	extern void UpdateActiveTemperature(size_t index, int ival);
	extern void UpdateStandbyTemperature(size_t index, int ival);
	extern void UpdateExtrusionFactor(size_t index, int ival);
	extern void UpdatePrintTimeText(uint32_t seconds, bool isSimulated);
	extern void UpdateSpeedPercent(int ival);
	extern void FirmwareFeaturesChanged(FirmwareFeatures newFeatures);
	extern void ProcessTouch(ButtonPress bp);
	extern void ProcessTouchOutsidePopup(ButtonPress bp)
	pre(bp.IsValid());
	extern void OnButtonPressTimeout();
	extern bool IsDisplayingFileInfo();
	extern void AllToolsSeen();

	extern void DisplayFilesOrMacrosList(bool filesNotMacros, int cardNumber, unsigned int numVolumes);
	extern void FileListLoaded(bool filesNotMacros, int errCode);
	extern void EnableFileNavButtons(bool filesNotMacros, bool scrollEarlier, bool scrollLater, bool parentDir);
	extern void UpdateFileButton(bool filesNotMacros, unsigned int buttonIndex, const char * array null text, const char * array null param);
	extern unsigned int GetNumScrolledFiles(bool filesNotMacros);
	extern bool UpdateMacroShortList(unsigned int buttonIndex, const char * array null fileName);

	extern void SetBabystepOffset(size_t index, float f);
	extern void SetAxisLetter(size_t index, char l);
	extern void SetAxisVisible(size_t index, bool v);

	extern void SetCurrentTool(int32_t tool);
	extern void UpdateToolStatus(size_t index, ToolStatus status);
	extern void RemoveTool(size_t index, bool allFollowing = false);
	extern void SetToolExtruder(size_t toolIndex, int8_t extruder);
	extern void SetToolHeater(size_t toolIndex, int8_t heater);
	extern void SetSpindleTool(int8_t toolIndex, int8_t spindle);

	extern void SetBedOrChamberHeater(int8_t heaterNumber, uint8_t heaterIndex, bool bed = true);

	extern void RemoveSpindle(size_t index, bool allFollowing = false);
	extern void SetSpindleActive(size_t index, float active);
	extern void SetSpindleCurrent(size_t index, float current);
	extern void SetSpindleMax(size_t index, float max);
}

#endif /* SRC_USERINTERFACE_HPP_ */
