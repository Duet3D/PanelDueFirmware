/*
 * ObjectModel.hpp
 *
 *  Created on: 7 Sep 2020
 *      Author: manuel
 */

#ifndef SRC_OBJECTMODEL_HPP_
#define SRC_OBJECTMODEL_HPP_

#include <cstdint>

//#undef array
//#undef vsnprintf
//#undef snprintf
//#include <functional>
//// Also reinstate the safeguards against using the wrong *snprintf versions
//#define vsnprintf(b, m, f, a) static_assert(false, "Do not use vsnprintf, use SafeVsnprintf instead")
//#define snprintf(b, m, f, ...) static_assert(false, "Do not use snprintf, use SafeSnprintf instead")

#include "ToolStatus.hpp"
#include "UserInterfaceConstants.hpp"
#include <General/FreelistManager.h>
#include <General/Vector.hpp>
#include <General/inplace_function.h>

#ifndef UNUSED
# define UNUSED(_x)	(void)(_x)
#endif

namespace OM {
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
		void* operator new(size_t sz) noexcept { UNUSED(sz); return FreelistManager::Allocate<Axis>(); }
		void operator delete(void* p) noexcept { FreelistManager::Release<Axis>(p); }

		uint8_t index;
		float babystep;
		char letter[2];
		float workplaceOffsets[9];
		uint8_t homed : 1,
			visible : 1,
			slot : 6;

		void Reset()
		{
			index = 0;
			babystep = 0.0f;
			letter[0] = 0;
			letter[1] = 0;
			for (size_t i = 0; i < MaxTotalWorkplaces; ++i)
			{
				workplaceOffsets[i] = 0.0f;
			}
			homed = false;
			visible = false;
			slot = MaxSlots;
		}
	};

	struct Spindle
	{
		void* operator new(size_t sz) noexcept { UNUSED(sz); return FreelistManager::Allocate<Spindle>(); }
		void operator delete(void* p) noexcept { FreelistManager::Release<Spindle>(p); }

		// Index within configured spindles
		uint8_t index;
		uint16_t active;
		uint16_t max;
		int8_t tool;

		void Reset()
		{
			index = 0;
			active = 0;
			max = 0;
			tool = -1;
		}
	};

	struct Tool
	{
		void* operator new(size_t sz) noexcept { UNUSED(sz); return FreelistManager::Allocate<Tool>(); }
		void operator delete(void* p) noexcept { FreelistManager::Release<Tool>(p); }

		// tool number
		uint8_t index;
		int8_t heater;				// only look at the first heater as we only display one
		int16_t activeTemp;
		int16_t standbyTemp;
		int8_t extruder;			// only look at the first extruder as we only display one
		Spindle* spindle;		// only look at the first spindle as we only display one
		int8_t fan;
		float offsets[MaxTotalAxes];
		ToolStatus status;
		uint8_t slot;

		void Reset()
		{
			index = 0;
			heater = -1;				// only look at the first heater as we only display one
			activeTemp = 0;
			standbyTemp = 0;
			extruder = -1;			// only look at the first extruder as we only display one
			spindle = nullptr;		// only look at the first spindle as we only display one
			fan = -1;
			for (size_t i = 0; i < MaxTotalAxes; ++i)
			{
				offsets[i] = 0.0f;
			}
			status = ToolStatus::off;
			slot = MaxSlots;
		}
	};

	struct BedOrChamber
	{
		void* operator new(size_t sz) noexcept { UNUSED(sz); return FreelistManager::Allocate<BedOrChamber>(); }
		void operator delete(void* p) noexcept { FreelistManager::Release<BedOrChamber>(p); }

		// Index within configured heaters
		uint8_t index = 0;
		// Id of heater
		int8_t heater = -1;
		// Slot for display on panel
		uint8_t slot = MaxSlots;

		void Reset()
		{
			index = 0;
			heater = -1;
			slot = MaxSlots;
		}
	};

	typedef BedOrChamber Bed;
	typedef BedOrChamber Chamber;

	typedef void (*AxisIterator)(Axis*);
	typedef bool (*AxisIteratorWhile)(Axis*);

	typedef Vector<uint8_t, MaxSlots> HeaterSlots;

	Axis* FindAxis(stdext::inplace_function<bool(Axis*)> filter);
	Axis* GetAxis(const size_t index);
	Axis* GetAxisInSlot(const size_t slot);
	Axis* GetOrCreateAxis(const size_t index);
	void IterateAxes(stdext::inplace_function<void(Axis*)> func, const size_t startAt = 0);
	bool IterateAxesWhile(stdext::inplace_function<bool(Axis*)> func, const size_t startAt = 0);

	Spindle* GetSpindle(const size_t index);
	Spindle* GetOrCreateSpindle(const size_t index);
	Spindle* GetSpindleForTool(const size_t toolNumber);
	void IterateSpindles(stdext::inplace_function<void(Spindle*)> func, const size_t startAt = 0);
	bool IterateSpindlesWhile(stdext::inplace_function<bool(Spindle*)> func, const size_t startAt = 0);

	Tool* GetTool(const size_t index);
	Tool* GetOrCreateTool(const size_t index);
	Tool* GetToolForExtruder(const size_t extruder);
	Tool* GetToolForFan(const size_t fan);
	Tool* GetToolForHeater(const size_t heater);
	void IterateTools(stdext::inplace_function<void(Tool*)> func, const size_t startAt = 0);
	bool IterateToolsWhile(stdext::inplace_function<bool(Tool*)> func, const size_t startAt = 0);

	Bed* GetBed(const size_t index);
	Bed* GetOrCreateBed(const size_t index);
	Bed* GetFirstBed();
	Bed* GetBedForHeater(const size_t heater);
	size_t GetBedCount();
	void IterateBeds(stdext::inplace_function<void(Bed*)> func, const size_t startAt = 0);

	Chamber* GetChamber(const size_t index);
	Chamber* GetOrCreateChamber(const size_t index);
	Chamber* GetFirstChamber();
	Chamber* GetChamberForHeater(const size_t heater);
	size_t GetChamberCount();
	void IterateChambers(stdext::inplace_function<void(Chamber*)> func, const size_t startAt = 0);

	void GetHeaterSlots(
			const size_t heaterIndex,
			HeaterSlots& heaterSlots,
			const bool addTools = true,
			const bool addBeds = true,
			const bool addChambers = true);

	size_t RemoveAxis(const size_t index, const bool allFollowing);
	size_t RemoveSpindle(const size_t index, const bool allFollowing);
	size_t RemoveTool(const size_t index, const bool allFollowing);
	size_t RemoveBed(const size_t index, const bool allFollowing);
	size_t RemoveChamber(const size_t index, const bool allFollowing);
}


#endif /* SRC_OBJECTMODEL_HPP_ */
