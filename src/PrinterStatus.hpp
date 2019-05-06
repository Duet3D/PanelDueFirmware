/*
 * PrinterStatus.hpp
 *
 *  Created on: 6 Jan 2017
 *      Author: David
 */

#ifndef SRC_PRINTERSTATUS_HPP_
#define SRC_PRINTERSTATUS_HPP_

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
	off = 12
};

const char * const StatusLetters = "IPSCABDRFTMO";	// status letter codes corresponding to the above, except for the first one

#endif /* SRC_PRINTERSTATUS_HPP_ */
