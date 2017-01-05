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

// Global functions in PanelDue.cpp that are called from elsewhere
extern void ProcessReceivedValue(const char id[], const char val[], int index);
extern void ProcessArrayLength(const char id[], int length);
extern void StartReceivedMessage();
extern void EndReceivedMessage();

// Global data in PanelDue.cpp that is used elsewhere
extern UTFT lcd;
extern MainWindow mgr;

#endif /* PANELDUE_H_ */