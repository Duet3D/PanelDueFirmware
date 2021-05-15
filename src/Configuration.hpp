/*
 * Configuration.hpp
 *
 * Created: 16/01/2015 13:18:16
 *  Author: David
 */


#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <cstdint>

#define DISPLAY_TYPE_ITDB02_43			(1)		// Itead 4.3 inch display (480 x 272) or alternative 4.3 inch display with 24-bit colour
#define DISPLAY_TYPE_ITDB02_50			(2)		// Itead 5.0 inch display (800 x 480) or alternative 5 or 7 inch display with 24-bit colour
#define DISPLAY_TYPE_ITDB02_70			(3)		// 7.0 inch display (800 x 480) with 18-bit colour
#define DISPLAY_TYPE_ER_50_70			(4)		// 5.0 or 7.0 inch East Rising display (800 x 480) with 24-bit colour
#define DISPLAY_TYPE_CPLD_70			(5)		// 7.0 inch CPLD display (800 x 480) with 24-bit colour

// Define DISPLAY_TYPE to be one of the above 5 types of display

#ifdef SCREEN_43
#define DISPLAY_TYPE	DISPLAY_TYPE_ITDB02_43
#define LARGE_FONT		(0)
#define IS_24BIT		(true)
#define IS_ER			(false)
#endif

#ifdef SCREEN_50
#define DISPLAY_TYPE	DISPLAY_TYPE_ITDB02_50
#define LARGE_FONT		(1)
#define IS_24BIT		(true)
#define IS_ER			(false)
#endif

#ifdef SCREEN_70
#define DISPLAY_TYPE	DISPLAY_TYPE_ITDB02_70
#define LARGE_FONT		(1)
#define IS_24BIT		(false)
#define IS_ER			(false)
#endif

#if defined(SCREEN_70E) || defined(SCREEN_50E)
#define DISPLAY_TYPE	DISPLAY_TYPE_ER_50_70
#define LARGE_FONT		(1)
#define IS_24BIT		(true)
#define IS_ER			(true)
#endif

#ifdef SCREEN_70CPLD
#define DISPLAY_TYPE	DISPLAY_TYPE_CPLD_70
#define LARGE_FONT		(1)
#define IS_24BIT		(true)
#define IS_ER			(false)
#endif

#define USE_CYRILLIC_CHARACTERS		(0)

const uint32_t DefaultBaudRate = 115200;
const uint32_t DimDisplayTimeout = 60000;				// dim this display after no activity for this number of milliseconds
const uint32_t DefaultScreensaverTimeout = 120000;		// enable screensaver after no activity for this number of milliseconds
const uint32_t ScreensaverMoveTime = 10000;				// Jog around screen saver text after this number of milliseconds
const uint32_t DefaultInfoTimeout = 5;					// information message timeout in seconds
const uint8_t DefaultBabystepAmountIndex = 1;			// default babystep amount of 0.02mm
const uint16_t DefaultFeedrate = 6000;					// default feedrate in mm/min

const uint32_t MinimumEncoderCommandInterval = 50;		// minimum time in milliseconds between serial commands sent due to encoder movement
const uint32_t MaxEncoderCommandLength = 64;			// maximum length of an encoder movement command

#endif /* CONFIGURATION_H_ */
