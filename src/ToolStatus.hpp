/*
 * ToolStatus.hpp
 *
 *  Created on: 26 Aug 2020
 *      Author: Manuel
 */

#ifndef SRC_TOOLSTATUS_HPP_
#define SRC_TOOLSTATUS_HPP_

// Status that a tool may report to us.
enum class ToolStatus
{
	off = 0,
	active = 1,
	standby = 2,
};

struct ToolStatusMapEntry
{
	const char* key;
	ToolStatus val;
};

// This table must be kept in case-insensitive alphabetical order of the search string.
const ToolStatusMapEntry toolStatusMap[] =
{
	{"active",	ToolStatus::active },
	{"off",		ToolStatus::off },
	{"standby",	ToolStatus::standby },
};

#endif /* SRC_TOOLSTATUS_HPP_ */
