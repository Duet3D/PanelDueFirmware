/*
 * FirmwareFeatures.hpp
 *
 *  Created on: 28 Jan 2017
 *      Author: David
 */

#ifndef SRC_FIRMWAREFEATURES_HPP_
#define SRC_FIRMWAREFEATURES_HPP_

// Firmware features bitmap definition
typedef uint32_t FirmwareFeatures;
const FirmwareFeatures noGcodesFolder = 0x0001;		// gcodes files are in 0:/ not 0:/gcodes
const FirmwareFeatures noStandbyTemps = 0x0002;		// firmware does not support separate tool active and standby temperatures
const FirmwareFeatures noG10Temps = 0x0004;			// firmware does not support using G10 to set temperatures
const FirmwareFeatures noDriveNumber = 0x0008;		// firmware does not handle drive numbers at the start of file paths
const FirmwareFeatures noM20M36 = 0x0010;			// firmware does not handle M20 S2 or M36 commands. Use M408 S20 and M408 S36 instead.
const FirmwareFeatures quoteFilenames = 0x0020;		// filenames should always be quoted in GCode commands

#endif /* SRC_FIRMWAREFEATURES_HPP_ */
