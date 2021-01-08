/*
 * ObjectModel.hpp
 *
 *  Created on: 7 Sep 2020
 *      Author: manuel
 */

#ifndef SRC_OBJECTMODEL_HPP_
#define SRC_OBJECTMODEL_HPP_

#include <cstdint>
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

		void Reset();
	};

	struct Spindle
	{
		void* operator new(size_t sz) noexcept { UNUSED(sz); return FreelistManager::Allocate<Spindle>(); }
		void operator delete(void* p) noexcept { FreelistManager::Release<Spindle>(p); }

		// Index within configured spindles
		uint8_t index;
		uint16_t active;
		uint16_t max;
		uint16_t min;
		int8_t tool;

		void Reset();
	};

	struct ToolHeater
	{
		void* operator new(size_t sz) noexcept { UNUSED(sz); return FreelistManager::Allocate<ToolHeater>(); }
		void operator delete(void* p) noexcept { FreelistManager::Release<ToolHeater>(p); }

		uint8_t index;	// This is the heater number
		int16_t activeTemp;
		int16_t standbyTemp;

		void Reset();
	};

	typedef Vector<ToolHeater*, MaxSlots> ToolHeaters;

	struct Tool
	{
		void* operator new(size_t sz) noexcept { UNUSED(sz); return FreelistManager::Allocate<Tool>(); }
		void operator delete(void* p) noexcept { FreelistManager::Release<Tool>(p); }

		// tool number
		uint8_t index;
		ToolHeaters heaters;
		int8_t extruder;			// only look at the first extruder as we only display one
		Spindle* spindle;			// only look at the first spindle as we only display one
		int8_t fan;
		float offsets[MaxTotalAxes];
		ToolStatus status;
		uint8_t slot;

		ToolHeater* GetOrCreateHeater(const uint8_t heaterIndex);
		int8_t HasHeater(const uint8_t heaterIndex) const;
		void IterateHeaters(stdext::inplace_function<void(ToolHeater*)> func, const size_t startAt = 0);
		size_t RemoveHeatersFrom(const uint8_t heaterIndex);
		void UpdateTemp(const uint8_t heaterIndex, const int32_t temp, const bool active);

		void Reset();
	};

	struct BedOrChamber
	{
		void* operator new(size_t sz) noexcept { UNUSED(sz); return FreelistManager::Allocate<BedOrChamber>(); }
		void operator delete(void* p) noexcept { FreelistManager::Release<BedOrChamber>(p); }

		// Index within configured heaters
		uint8_t index;
		// Id of heater
		int8_t heater;
		// Slot for display on panel
		uint8_t slot;

		void Reset();
	};

	typedef BedOrChamber Bed;
	typedef BedOrChamber Chamber;

	typedef Vector<uint8_t, MaxSlots> HeaterSlots;

	Axis* GetAxis(const size_t index);
	Axis* GetOrCreateAxis(const size_t index);
	void IterateAxes(stdext::inplace_function<void(Axis*)> func, const size_t startAt = 0);
	bool IterateAxesWhile(stdext::inplace_function<bool(Axis*)> func, const size_t startAt = 0);
	size_t RemoveAxis(const size_t index, const bool allFollowing);

	Spindle* GetSpindle(const size_t index);
	Spindle* GetOrCreateSpindle(const size_t index);
	size_t RemoveSpindle(const size_t index, const bool allFollowing);

	Tool* GetTool(const size_t index);
	Tool* GetOrCreateTool(const size_t index);
	Tool* GetToolForExtruder(const size_t extruder);
	Tool* GetToolForFan(const size_t fan);
	void IterateTools(stdext::inplace_function<void(Tool*)> func, const size_t startAt = 0);
	size_t RemoveTool(const size_t index, const bool allFollowing);

	Bed* GetOrCreateBed(const size_t index);
	Bed* GetFirstBed();
	Bed* GetBedForHeater(const size_t heater);
	size_t GetBedCount();
	void IterateBeds(stdext::inplace_function<void(Bed*)> func, const size_t startAt = 0);
	size_t RemoveBed(const size_t index, const bool allFollowing);

	Chamber* GetOrCreateChamber(const size_t index);
	Chamber* GetFirstChamber();
	Chamber* GetChamberForHeater(const size_t heater);
	size_t GetChamberCount();
	void IterateChambers(stdext::inplace_function<void(Chamber*)> func, const size_t startAt = 0);
	size_t RemoveChamber(const size_t index, const bool allFollowing);

	void GetHeaterSlots(
			const size_t heaterIndex,
			HeaterSlots& heaterSlots,
			const bool addTools = true,
			const bool addBeds = true,
			const bool addChambers = true);
}


#endif /* SRC_OBJECTMODEL_HPP_ */
