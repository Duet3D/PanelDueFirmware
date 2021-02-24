/*
 * FirmwareFeatures.hpp
 *
 *  Created on: 28 Jan 2017
 *      Author: David
 */

#ifndef SRC_FIRMWAREFEATURES_HPP_
#define SRC_FIRMWAREFEATURES_HPP_

#include <General/Bitmap.h>

// Firmware features bitmap definition
typedef Bitmap<uint8_t> FirmwareFeatureMap;
enum FirmwareFeatures : uint8_t
{
	noGcodesFolder = 0,		// gcodes files are in 0:/ not 0:/gcodes
	noStandbyTemps,			// firmware does not support separate tool active and standby temperatures
	noG10Temps,				// firmware does not support using G10 to set temperatures
	noDriveNumber,			// firmware does not handle drive numbers at the start of file paths
	noM20M36,				// firmware does not handle M20 S2 or M36 commands. Use M408 S20 and M408 S36 instead.
	quoteFilenames,			// filenames should always be quoted in GCode commands
	m568TempAndRPM,			// firmware supports M568 to set tool temps and tool spindle RPM
};

#endif /* SRC_FIRMWAREFEATURES_HPP_ */
