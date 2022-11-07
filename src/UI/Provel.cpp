/*
 * Provel.cpp
 *
 *  Created on: 07-11-2022
 *      Author: Michael Szafranek (faule.aepfel@gmail.com)
 */

// TODO
// user interface
//
// - generic screen
//   - 1 column
//   - 6 rows
//     - static/dynamic text fields
//     - button
//       - half width
//       - full full
//
// - screens
//   - splash
//   - homing
//   - loading file
//   - file loaded
//   - idle
//   - printing
//   - heating
//   - z calibrate
//   - purging
//   - printer
//   - fault
//   - warning

#include "UI/Provel.hpp"

#include "Configuration.hpp"
#include "FileManager.hpp"
#include "FlashData.hpp"

#include "General/SafeVsnprintf.h"
#include "General/SimpleMath.h"
#include "General/String.h"
#include "General/StringFunctions.h"

#include "Hardware/Buzzer.hpp"
#include "Hardware/Reset.hpp"
#include "Hardware/SerialIo.hpp"
#include "Hardware/SysTick.hpp"

#include "Icons/Icons.hpp"
#include "Library/Misc.hpp"

#include "ObjectModel/Axis.hpp"
#include "ObjectModel/PrinterStatus.hpp"
#include "ObjectModel/Utils.hpp"

#include "UI/Display.hpp"
#include "UI/MessageLog.hpp"
#include "UI/Popup.hpp"
#include "UI/Strings.hpp"
#include "UI/UserInterface.hpp"
#include "UI/UserInterfaceConstants.hpp"
#include "Version.hpp"

#define DEBUG 0
#include "Debug.hpp"

static MainWindow *Provel;

