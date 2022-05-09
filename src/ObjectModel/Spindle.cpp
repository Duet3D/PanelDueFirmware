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

#include <cstdlib>

typedef Vector<OM::Spindle*, MaxSlots> SpindleList;
static SpindleList spindles;

namespace OM
{
	void Spindle::Reset()
	{
		index = 0;
		active = 0;
		current = 0;
		max = 16000;
		min = -16000;
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

	void Spindle::SetActive(size_t spindleIndex, int32_t activeRpm)
	{
		auto spindle = OM::GetOrCreateSpindle(spindleIndex);
		if (spindle == nullptr)
		{
			return;
		}

		// hack to keep current rotation direction
		spindle->active = ((spindle->active > 0) ? 1 : -1) * activeRpm;
	}

	void Spindle::SetCurrent(size_t spindleIndex, int32_t current)
	{
		auto spindle = OM::GetOrCreateSpindle(spindleIndex);
		if (spindle == nullptr)
		{
			return;
		}

		spindle->current = ((spindle->current > 0) ? 1 : -1) * current;
	}

	void Spindle::SetLimitMax(size_t spindleIndex, int32_t max)
	{
		OM::Spindle *spindle = OM::GetOrCreateSpindle(spindleIndex);
		if (spindle == nullptr)
		{
			return;
		}

		spindle->max = max;
	}

	void Spindle::SetLimitMin(size_t spindleIndex, int32_t min)
	{
		OM::Spindle *spindle = OM::GetOrCreateSpindle(spindleIndex);
		if (spindle == nullptr)
		{
			return;
		}

		spindle->min = min;
	}

	void Spindle::SetState(size_t spindleIndex, OM::SpindleState state)
	{
		OM::Spindle* spindle = OM::GetOrCreateSpindle(spindleIndex);
		if (spindle == nullptr)
		{
			return;
		}

		switch (state)
		{
		case OM::SpindleState::forward:
			spindle->current = abs(spindle->current);
			spindle->active = abs(spindle->active);
			break;
		case OM::SpindleState::reverse:
			spindle->current = -1 * abs(spindle->current);
			spindle->active = -1 * abs(spindle->active);
			break;
		case OM::SpindleState::stopped:
			spindle->current = 0;
			spindle->active = 0;
			break;
		default:
			break;
		}
	}
}
