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
	extern void ActivateScreensaver();
	extern void DeactivateScreensaver();
	extern void AnimateScreensaver();
	extern void ShowAxis(size_t axis, bool b, char axisLetter =  '\0');
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
	extern void LastJobFileNameAvailable(const bool available);
	extern void SetLastFileSimulated(const bool lastFileSimulated);
	extern void ShowDefaultPage();
	extern void UpdatePrintingFields();
	extern void SetPrintProgressPercent(unsigned int percent);
	extern void UpdateGeometry(unsigned int p_numAxes, bool p_isDelta);
	extern void UpdateHomedStatus(size_t axis, bool isHomed);
	extern void UpdateZProbe(const char data[]);
	extern void UpdateMachineName(const char data[]);
	extern void ProcessAlert(const Alert& alert);
	extern void ClearAlert();
	extern void ProcessSimpleAlert(const char* _ecv_array text);
	extern void NewResponseReceived(const char* _ecv_array text);
	extern bool CanDimDisplay();
	extern void UpdateFileLastModifiedText(const char data[]);
	extern void UpdateFileGeneratedByText(const char data[]);
	extern void UpdateFileObjectHeight(float f);
	extern void UpdateFileLayerHeight(float f);
	extern void UpdateFileSize(int size);
	extern void UpdateFileFilament(int len);
	extern void UpdateFanPercent(size_t fanIndex, int rpm);
	extern void UpdateActiveTemperature(size_t index, int ival);
	extern void UpdateToolTemp(size_t toolIndex, size_t toolHeaterIndex, int32_t temp, bool active);
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
	extern void UpdateFileButton(bool filesNotMacros, unsigned int buttonIndex, const char * _ecv_array null text, const char * _ecv_array null param);
	extern unsigned int GetNumScrolledFiles(bool filesNotMacros);
	extern bool UpdateMacroShortList(unsigned int buttonIndex, const char * _ecv_array null fileName);

	extern void SetBabystepOffset(size_t index, float f);
	extern void SetAxisLetter(size_t index, char l);
	extern void SetAxisVisible(size_t index, bool v);
	extern void SetAxisWorkplaceOffset(size_t axisIndex, size_t workplaceIndex, float offset);

	extern void SetCurrentTool(int32_t tool);
	extern void UpdateToolStatus(size_t index, ToolStatus status);
	extern void SetToolExtruder(size_t toolIndex, int8_t extruder);
	extern void SetToolFan(size_t toolIndex, int8_t fan);
	extern void SetToolHeater(size_t toolIndex, uint8_t toolHeaterIndex, int8_t heaterIndex);
	extern bool RemoveToolHeaters(const size_t toolIndex, const uint8_t firstIndexToDelete);
	extern void SetToolOffset(size_t toolIndex, size_t axisIndex, float offset);

	extern void SetBedOrChamberHeater(const uint8_t heaterIndex, const int8_t heaterNumber, bool bed = true);

	extern void SetSpindleActive(size_t index, uint16_t active);
	extern void SetSpindleCurrent(size_t index, uint16_t current);
	extern void SetSpindleLimit(size_t index, uint16_t value, bool max);
	extern void SetSpindleTool(int8_t spindle, int8_t toolIndex);
}

#endif /* SRC_USERINTERFACE_HPP_ */
