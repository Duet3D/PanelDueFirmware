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
#include "FirmwareFeatures.hpp"
#include "FlashData.hpp"
#include <ObjectModel/PrinterStatus.hpp>
#include <General/String.h>

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

const size_t MIN_AXES = 2;					// the minimum number of axes we support

#endif /* PANELDUE_H_ */
