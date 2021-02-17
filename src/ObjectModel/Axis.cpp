/*
 * Axis.cpp
 *
 *  Created on: 17 Feb 2021
 *      Author: manuel
 */

#include "Axis.hpp"
#include "UserInterfaceConstants.hpp"
#include "ListHelpers.hpp"
#include <General/Vector.hpp>

typedef Vector<OM::Axis*, MaxTotalAxes> AxisList;
static AxisList axes;

namespace OM
{
	void OM::Axis::Reset()
	{
		index = 0;
		babystep = 0.0f;
		letter[0] = 0;
		letter[1] = 0;
		for (size_t i = 0; i < Workplaces::MaxTotalWorkplaces; ++i)
		{
			workplaceOffsets[i] = 0.0f;
		}
		homed = false;
		visible = false;
		slot = MaxSlots;
	}

	Axis* GetAxis(const size_t index)
	{
		if (index >= MaxTotalAxes)
		{
			return nullptr;
		}
		return GetOrCreate<AxisList, Axis>(axes, index, false);
	}

	Axis* GetOrCreateAxis(const size_t index)
	{
		if (index >= MaxTotalAxes)
		{
			return nullptr;
		}
		return GetOrCreate<AxisList, Axis>(axes, index, true);
	}

	void IterateAxes(stdext::inplace_function<void(Axis*)> func, const size_t startAt)
	{
		Iterate(axes, func, startAt);
	}

	bool IterateAxesWhile(stdext::inplace_function<bool(Axis*)> func, const size_t startAt)
	{
		return IterateWhile(axes, func, startAt);
	}

	size_t RemoveAxis(const size_t index, const bool allFollowing)
	{
		return Remove<AxisList, Axis>(axes, index, allFollowing);
	}
}
