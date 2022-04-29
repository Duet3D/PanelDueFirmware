/*
 * Spindle.cpp
 *
 *  Created on: 17 Feb 2021
 *      Author: manuel
 */

#include "Spindle.hpp"
#include "ListHelpers.hpp"
#include <General/Vector.hpp>
#include <UI/UserInterfaceConstants.hpp>

typedef Vector<OM::Spindle*, MaxSlots> SpindleList;
static SpindleList spindles;

namespace OM
{
	void Spindle::Reset()
	{
		index = 0;
		active = 0;
		current = 0;
		max = 10000;
		min = 0;
	}

	Spindle* GetSpindle(const size_t index)
	{
		return GetOrCreate<SpindleList, Spindle>(spindles, index, false);
	}

	Spindle* GetOrCreateSpindle(const size_t index)
	{
		return GetOrCreate<SpindleList, Spindle>(spindles, index, true);
	}

	size_t RemoveSpindle(const size_t index, const bool allFollowing)
	{
		return Remove<SpindleList, Spindle>(spindles, index, allFollowing);
	}
}
