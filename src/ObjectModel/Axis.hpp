/*
 * Axis.hpp
 *
 *  Created on: 17 Feb 2021
 *      Author: manuel
 */

#ifndef SRC_OBJECTMODEL_AXIS_HPP_
#define SRC_OBJECTMODEL_AXIS_HPP_

#include <cstdint>
#include <General/FreelistManager.h>
#include <General/function_ref.h>

namespace OM
{
	enum Workplaces
	{
		G54,
		G55,
		G56,
		G57,
		G58,
		G59,
		G59_1,
		G59_2,
		G59_3,
		MaxTotalWorkplaces
	};

	struct Axis
	{
		void* operator new(size_t) noexcept { return FreelistManager::Allocate<Axis>(); }
		void operator delete(void* p) noexcept { FreelistManager::Release<Axis>(p); }

		uint8_t index;
		float babystep;
		char letter[2];
		float workplaceOffsets[9];
		uint16_t homed : 1,
			visible : 1,
			slot : 6,
			slotP : 6;

		void Reset();
	};

	Axis* GetAxis(const size_t index);
	Axis* GetOrCreateAxis(const size_t index);
	bool IterateAxesWhile(function_ref<bool(Axis*&, size_t)> func, const size_t startAt = 0);
	size_t RemoveAxis(const size_t index, const bool allFollowing);
}


#endif /* SRC_OBJECTMODEL_AXIS_HPP_ */
