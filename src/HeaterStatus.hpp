/*
 * HeaterStatus.hpp
 *
 *  Created on: 11 Aug 2020
 *      Author: manuel
 */

#ifndef SRC_HEATERSTATUS_HPP_
#define SRC_HEATERSTATUS_HPP_

enum class HeaterStatus
{
	off = 0,
	standby,
	active,
	fault,
	tuning,
	offline
};


struct HeaterStatusMapEntry
{
	const char* key;
	const HeaterStatus val;
};

// This table has to be kept in alphabetical order of the keys
const HeaterStatusMapEntry heaterStatusMap[] =
{
	{ "active",		HeaterStatus::active },
	{ "fault",		HeaterStatus::fault },
	{ "off",	 	HeaterStatus::off },
	{ "offline",	HeaterStatus::offline },
	{ "standby",	HeaterStatus::standby },
	{ "tuning",		HeaterStatus::tuning },
};

#endif /* SRC_HEATERSTATUS_HPP_ */
