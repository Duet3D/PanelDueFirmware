/*
 * ObjectModel.hpp
 *
 *  Created on: 7 Sep 2020
 *      Author: manuel
 */

#ifndef SRC_OBJECTMODEL_HPP_
#define SRC_OBJECTMODEL_HPP_

#include <cstdint>

#undef array
#undef vsnprintf
#undef snprintf
#include <functional>
// Also reinstate the safeguards against using the wrong *snprintf versions
#define vsnprintf(b, m, f, a) static_assert(false, "Do not use vsnprintf, use SafeVsnprintf instead")
#define snprintf(b, m, f, ...) static_assert(false, "Do not use snprintf, use SafeSnprintf instead")

#include "ToolStatus.hpp"
#include "UserInterfaceConstants.hpp"

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
		uint8_t index = 0;
		float babystep = 0.0f;
		char letter[2] = {'\0', '\0'};
		float workplaceOffsets[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
		uint8_t homed : 1,
			visible : 1,
			slot : 6;
		Axis* next = nullptr;
	};

	struct Spindle
	{
		// Index within configured spindles
		uint8_t index = 0;
		uint16_t active = 0;
		uint16_t max = 0;
		int8_t tool = -1;
		Spindle* next = nullptr;
	};

	struct Tool
	{
		// tool number
		uint8_t index = 0;
		int8_t heater = -1;				// only look at the first heater as we only display one
		int8_t extruder = -1;			// only look at the first extruder as we only display one
		Spindle* spindle = nullptr;		// only look at the first spindle as we only display one
		float offsets[MaxTotalAxes];
		ToolStatus status = ToolStatus::off;
		uint8_t slot = MaxHeaters;
		Tool* next = nullptr;
	};

	struct BedOrChamber
	{
		// Index within configured heaters
		uint8_t index = 0;
		// Id of heater
		int8_t heater = -1;
		// Slot for display on panel
		uint8_t slot = MaxHeaters;

		BedOrChamber* next = nullptr;

		void Reset() { index = 0; heater = -1; slot = MaxHeaters; }
	};

	typedef BedOrChamber Bed;
	typedef BedOrChamber Chamber;

	typedef void (*AxisIterator)(Axis*);
	typedef bool (*AxisIteratorWhile)(Axis*);

	Axis* FindAxis(std::function<bool(Axis*)> filter);
	Axis* GetAxis(const size_t index);
	Axis* GetAxisInSlot(const size_t slot);
	Axis* GetOrCreateAxis(const size_t index);
	void IterateAxes(std::function<void(Axis*)> func, const size_t startAt = 0);
	bool IterateAxesWhile(std::function<bool(Axis*)> func, const size_t startAt = 0);

	Spindle* GetSpindle(const size_t index);
	Spindle* GetOrCreateSpindle(const size_t index);
	Spindle* GetSpindleForTool(const size_t toolNumber);
	void IterateSpindles(std::function<void(Spindle*)> func, const size_t startAt = 0);
	bool IterateSpindlesWhile(std::function<bool(Spindle*)> func, const size_t startAt = 0);

	Tool* GetTool(const size_t index);
	Tool* GetOrCreateTool(const size_t index);
	Tool* GetToolForExtruder(const size_t extruder);
	Tool* GetToolForHeater(const size_t heater);
	void IterateTools(std::function<void(Tool*)> func, const size_t startAt = 0);
	bool IterateToolsWhile(std::function<bool(Tool*)> func, const size_t startAt = 0);

	Bed* GetBed(const size_t index);
	Bed* GetOrCreateBed(const size_t index);
	Bed* GetFirstBed();
	Bed* GetBedForHeater(const size_t heater);
	size_t GetBedCount();
	void IterateBeds(std::function<void(Bed*)> func, const size_t startAt = 0);

	Chamber* GetChamber(const size_t index);
	Chamber* GetOrCreateChamber(const size_t index);
	Chamber* GetFirstChamber();
	Chamber* GetChamberForHeater(const size_t heater);
	size_t GetChamberCount();
	void IterateChambers(std::function<void(Chamber*)> func, const size_t startAt = 0);

	size_t RemoveAxis(const size_t index, const bool allFollowing);
	size_t RemoveSpindle(const size_t index, const bool allFollowing);
	size_t RemoveTool(const size_t index, const bool allFollowing);
	size_t RemoveBed(const size_t index, const bool allFollowing);
	size_t RemoveChamber(const size_t index, const bool allFollowing);
}


#endif /* SRC_OBJECTMODEL_HPP_ */
