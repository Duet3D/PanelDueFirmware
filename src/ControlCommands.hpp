/*
 * ControlCommands.hpp
 *
 *  Created on: 4 Nov 2020
 *      Author: manuel
 */

#ifndef SRC_CONTROLCOMMANDS_HPP_
#define SRC_CONTROLCOMMANDS_HPP_

enum class ControlCommand
{
	invalid,
	reset,
	eraseAndReset,
};


struct ControlCommandMapEntry
{
	const char* key;
	const ControlCommand val;
};

// This table has to be kept in alphabetical order of the keys
const ControlCommandMapEntry controlCommandMap[] =
{
	{ "eraseAndReset",	ControlCommand::eraseAndReset },
	{ "reset",			ControlCommand::reset },
};

#endif /* SRC_CONTROLCOMMANDS_HPP_ */
