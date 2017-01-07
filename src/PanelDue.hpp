/*
 * PanelDue.hpp
 *
 * Created: 06/12/2014 14:23:38
 *  Author: David
 */ 

#ifndef PANELDUE_H_
#define PANELDUE_H_

#include "Hardware/UTFT.hpp"
#include "Display.hpp"
#include "RequestTimer.hpp"
#include "PrinterStatus.hpp"

// Functions called from the serial I/O module
extern void ProcessReceivedValue(const char id[], const char val[], int index);
extern void ProcessArrayLength(const char id[], int length);
extern void StartReceivedMessage();
extern void EndReceivedMessage();

// Functions called from module UserInterface
extern bool PrintInProgress();
extern PrinterStatus GetStatus();
extern void DelayTouchLong();
extern void ShortenTouchDelay();
extern void TouchBeep();
extern void ErrorBeep();
extern void CalibrateTouch();

// Functions called from module UserInterface to manipulate non-volatile settings and associated hardware
extern void FactoryReset();
extern void SaveSettings();
extern void CheckSettingsAreSaved();
extern bool HasLanguageChanged();
extern bool HaveSettingsChanged();
extern void MirrorDisplay();
extern void InvertDisplay();
extern void SetBaudRate(uint32_t rate);
extern void ChangeBrightness(bool up);
extern void SetVolume(uint32_t newVolume);
extern void SetColourScheme(uint32_t newColours);
extern void SetLanguage(uint32_t newLanguage);
extern uint32_t GetBaudRate();
extern uint32_t GetVolume();

// Global data in PanelDue.cpp that is used elsewhere
extern UTFT lcd;
extern MainWindow mgr;

class ColourScheme;
extern const ColourScheme *colours;

#endif /* PANELDUE_H_ */
