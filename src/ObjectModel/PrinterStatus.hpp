/*
 * PrinterStatus.hpp
 *
 *  Created on: 6 Jan 2017
 *      Author: David
 */

#ifndef SRC_OBJECTMODEL_PRINTERSTATUS_HPP_
#define SRC_OBJECTMODEL_PRINTERSTATUS_HPP_


namespace OM {
	// Status that the printer may report to us.
	// The array 'statusText' must be kept in sync with this!
	enum class PrinterStatus
	{
		connecting = 0,
		idle = 1,
		printing = 2,
		stopped = 3,
		configuring = 4,
		paused = 5,
		busy = 6,
		pausing = 7,
		resuming = 8,
		flashing = 9,
		toolChange = 10,
		simulating = 11,
		off = 12,
		cancelling = 13,
		NumTypes
	};

	struct PrinterStatusMapEntry
	{
		const char* key;
		PrinterStatus val;
	};

	// This table must be kept in case-insensitive alphabetical order of the search string.
	const PrinterStatusMapEntry printerStatusMap[] =
	{
		{"busy",		PrinterStatus::busy },
		{"cancelling",		PrinterStatus::cancelling },
		{"changingTool",	PrinterStatus::toolChange },
		{"halted",		PrinterStatus::stopped },
		{"idle",		PrinterStatus::idle },
		{"off",			PrinterStatus::off },
		{"paused",		PrinterStatus::paused },
		{"pausing",		PrinterStatus::pausing },
		{"processing",		PrinterStatus::printing },
		{"resuming",		PrinterStatus::resuming },
		{"simulating",		PrinterStatus::simulating },
		{"starting",		PrinterStatus::configuring },
		{"updating",		PrinterStatus::flashing },
	};
}

#endif /* SRC_OBJECTMODEL_PRINTERSTATUS_HPP_ */
