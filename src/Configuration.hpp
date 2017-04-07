/*
 * Configuration.hpp
 *
 * Created: 16/01/2015 13:18:16
 *  Author: David
 */ 


#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <cstdint>

#define VERSION_TEXT		"1.16"

#define DISPLAY_TYPE_ITDB02_43			(1)		// Itead 4.3 inch display (480 x 272) or alternative 4.3 inch display with 24-bit colour
#define DISPLAY_TYPE_ITDB02_50			(2)		// Itead 5.0 inch display (800 x 480) or alternative 5 or 7 inch display with 24-bit colour
#define DISPLAY_TYPE_ITDB02_70			(3)		// 7.0 inch display (800 x 480) with 18-bit colour

// Define DISPLAY_TYPE to be one of the above 3 types of display

#ifdef SCREEN_43
#define DISPLAY_TYPE	DISPLAY_TYPE_ITDB02_43
#define LARGE_FONT		(0)
#endif

#ifdef SCREEN_50
#define DISPLAY_TYPE	DISPLAY_TYPE_ITDB02_50
#define LARGE_FONT		(1)
#endif

#ifdef SCREEN_70
#define DISPLAY_TYPE	DISPLAY_TYPE_ITDB02_70
#define LARGE_FONT		(1)
#endif

const uint32_t DefaultBaudRate = 57600;

#endif /* CONFIGURATION_H_ */
