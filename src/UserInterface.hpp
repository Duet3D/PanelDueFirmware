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

extern IntegerField *freeMem;
extern TextButton *filenameButtons[];
extern StaticTextField *debugField;
extern StaticTextField *touchCalibInstruction;
extern StaticTextField *messageTextFields[], *messageTimeFields[];
extern TextField *fwVersionField;

namespace UI
{
	extern unsigned int GetNumLanguages();
	extern void CreateFields(uint32_t language, const ColourScheme& colours);
	extern void CheckSettingsAreSaved();
	extern void ShowAxis(size_t axis, bool b);
	extern void UpdateAxisPosition(size_t axis, float fval);
	extern void UpdateCurrentTemperature(size_t heater, float fval) pre(heater < maxHeaters);
	extern void ShowHeater(size_t heater, bool show) pre(heater < maxHeaters);
	extern void UpdateHeaterStatus(size_t heater, int ival) pre(heater < maxHeaters);
	extern void ChangeStatus(PrinterStatus oldStatus, PrinterStatus newStatus);
	extern void UpdateTimesLeft(size_t index, unsigned int seconds);
	extern bool ChangePage(ButtonBase *newTab);
	extern bool DoPolling();
	extern void Spin();
	extern void PrintStarted();
	extern void PrintingFilenameChanged(const char data[]);
	extern void ShowDefaultPage();
	extern void UpdatePrintingFields();
	extern void SetPrintProgressPercent(unsigned int percent);
	extern void UpdateGeometry(unsigned int numAxes, bool isDelta);
	extern void UpdateHomedStatus(int axis, bool isHomed);
	extern void UpdateZProbe(const char data[]);
	extern void UpdateMachineName(const char data[]);
	extern void ProcessAlert(const char data[]);
	extern void UpdateFileGeneratedByText(const char data[]);
	extern void UpdateFileObjectHeight(float f);
	extern void UpdateFileLayerHeight(float f);
	extern void UpdateFileSize(int size);
	extern void UpdateFileFilament(int len);
	extern void UpdateFanPercent(int rpm);
	extern void UpdateActiveTemperature(size_t index, int ival) pre(index < maxHeaters);
	extern void UpdateStandbyTemperature(size_t index, int ival) pre(index < maxHeaters);
	extern void UpdateExtrusionFactor(size_t index, int ival) pre(index + 1 < maxHeaters);
	extern void UpdateSpeedPercent(int ival);
	extern void FirmwareFeaturesChanged(FirmwareFeatures newFeatures);
	extern void ProcessTouch(ButtonPress bp);
	extern void ProcessTouchOutsidePopup(ButtonPress bp)
	pre(bp.IsValid());
	extern void OnButtonPressTimeout();
	extern bool IsDisplayingFileInfo();
	extern void UpdateFilesListTitle(int cardNumber, unsigned int numVolumes, bool isFilesList);
	extern void SetNumTools(unsigned int n);
	extern void FileListLoaded(int errCode);
	extern void EnableFileNavButtons(bool scrollEarlier, bool scrollLater, bool parentDir);
	extern unsigned int GetNumScrolledFiles();
	extern void SetBabystepOffset(float f);
}

#endif /* SRC_USERINTERFACE_HPP_ */
