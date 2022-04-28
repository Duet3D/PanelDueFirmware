/*
 * Spindle.hpp
 *
 *  Created on: 17 Feb 2021
 *      Author: manuel
 */

#ifndef SRC_OBJECTMODEL_SPINDLE_HPP_
#define SRC_OBJECTMODEL_SPINDLE_HPP_

#include <cstdint>
#include <General/FreelistManager.h>

namespace OM
{
	enum SpindleState : uint8_t {
		stopped, forward, reverse
	};

	struct SpindleStateMapEntry
	{
		const char* key;
		const SpindleState val;
	};

	// This table has to be kept in alphabetical order of the keys
	const SpindleStateMapEntry spindleStateMap[] =
	{
		{ "forward",	SpindleState::forward },
		{ "reverse",	SpindleState::reverse },
		{ "stopped",	SpindleState::stopped },
	};

	struct Spindle
	{
		void* operator new(size_t) noexcept { return FreelistManager::Allocate<Spindle>(); }
		void operator delete(void* p) noexcept { FreelistManager::Release<Spindle>(p); }

		// Index within configured spindles
		uint8_t index;
		int32_t active;
		int32_t current;
		int32_t max;
		int32_t min;
		SpindleState state;

		void Reset();
	};

	Spindle* GetSpindle(const size_t index);
	Spindle* GetOrCreateSpindle(const size_t index);
	size_t RemoveSpindle(const size_t index, const bool allFollowing);
}

#endif /* SRC_OBJECTMODEL_SPINDLE_HPP_ */
