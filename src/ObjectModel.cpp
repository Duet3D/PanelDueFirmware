/*
 * ObjectModel.cpp
 *
 *  Created on: 7 Sep 2020
 *      Author: manuel
 */

#include "ObjectModel.hpp"

typedef Vector<OM::Axis*, MaxTotalAxes> AxisList;
typedef Vector<OM::Spindle*, MaxSlots> SpindleList;
typedef Vector<OM::Tool*, MaxSlots> ToolList;
typedef Vector<OM::Bed*, MaxSlots> BedList;
typedef Vector<OM::Chamber*, MaxSlots> ChamberList;

static AxisList axes;
static SpindleList spindles;
static ToolList tools;
static BedList beds;
static ChamberList chambers;

template<typename L, typename T>
T* GetOrCreate(L& list, size_t index, bool create)
{
	const size_t count = list.Size();
	for (size_t i = 0; i < count; ++i)
	{
		if (list[i]->index == index)
		{
			return list[i];
		}
	}

	if (create)
	{
		T* elem = new T;
		elem->Reset();
		elem->index = index;
		list.Add(elem);
		list.Sort([] (T* e1, T* e2) { return e1->index > e2->index; });
		return elem;
	}

	return nullptr;
}

template<typename L, typename T>
T* Find(L& list, stdext::inplace_function<bool(T*)> filter)
{
	const size_t count = list.Size();
	for (size_t i = 0; i < count; ++i)
	{
		if (filter(list[i]))
		{
			return list[i];
		}
	}
	return nullptr;
}

template<typename L, typename T>
void Iterate(L& list, stdext::inplace_function<void(T*)> func, const size_t startAt)
{
	const size_t count = list.Size();
	for (size_t i = 0; i < count; ++i)
	{
		if (list[i]->index >= startAt)
		{
			func(list[i]);
		}
	}
}

template<typename L, typename T>
bool IterateWhile(L& list, stdext::inplace_function<bool(T*)> func, const size_t startAt)
{
	const size_t count = list.Size();
	for (size_t i = 0; i < count; ++i)
	{
		if (list[i]->index >= startAt)
		{
			if(!func(list[i]))
			{
				return false;
			}
		}
	}
	return true;
}

template<typename L, typename T>
size_t Remove(L& list, size_t index, bool allFollowing)
{
	size_t removed = 0;
	// Nothing to do on an empty list
	if (list.IsEmpty())
	{
		return removed;
	}

	const size_t count = list.Size();
	for (size_t i = count - 1; i > 0; --i)
	{
		T* elem = list[i];
		if (elem->index == index || (allFollowing && elem->index > index))
		{
			delete elem;
			list.Erase(i);
			++removed;
			if (!allFollowing)
			{
				break;
			}
		}
	}
	return removed;
}

namespace OM
{

	Axis* FindAxis(stdext::inplace_function<bool(Axis*)> filter)
	{
		return Find(axes, filter);
	}

	Axis* GetAxis(const size_t index)
	{
		if (index >= MaxTotalAxes)
		{
			return nullptr;
		}
		return GetOrCreate<AxisList, Axis>(axes, index, false);
	}

	Axis* GetAxisInSlot(const size_t slot)
	{
		if (slot >= MaxTotalAxes)
		{
			return nullptr;
		}
		return Find<AxisList, Axis>(axes, [slot](Axis* axis) { return axis->slot == slot; });
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

	Spindle* GetSpindle(const size_t index)
	{
		return GetOrCreate<SpindleList, Spindle>(spindles, index, false);
	}

	Spindle* GetOrCreateSpindle(const size_t index)
	{
		return GetOrCreate<SpindleList, Spindle>(spindles, index, true);
	}

	void IterateSpindles(stdext::inplace_function<void(Spindle*)> func, const size_t startAt)
	{
		Iterate(spindles, func, startAt);
	}

	bool IterateSpindlesWhile(stdext::inplace_function<bool(Spindle*)> func, const size_t startAt)
	{
		return IterateWhile(spindles, func, startAt);
	}

	Tool* GetTool(const size_t index)
	{
		return GetOrCreate<ToolList, Tool>(tools, index, false);
	}

	Tool* GetOrCreateTool(const size_t index)
	{
		return GetOrCreate<ToolList, Tool>(tools, index, true);
	}

	void IterateTools(stdext::inplace_function<void(Tool*)> func, const size_t startAt)
	{
		Iterate(tools, func, startAt);
	}

	bool IterateToolsWhile(stdext::inplace_function<bool(Tool*)> func, const size_t startAt)
	{
		return IterateWhile(tools, func, startAt);
	}

	Spindle* GetSpindleForTool(const size_t toolNumber)
	{
		return Find<SpindleList, Spindle>(spindles, [toolNumber](Spindle* spindle) { return spindle->tool == (int)toolNumber; });
	}

	Tool* GetToolForExtruder(const size_t extruder)
	{
		return Find<ToolList, Tool>(tools, [extruder](Tool* tool) { return tool->extruder == (int)extruder; });
	}

	Tool* GetToolForHeater(const size_t heater)
	{
		return Find<ToolList, Tool>(tools, [heater](Tool* tool) { return tool->heater == (int)heater; });
	}

	Bed* GetBed(const size_t index)
	{
		return GetOrCreate<BedList, Bed>(beds, index, false);
	}

	Bed* GetOrCreateBed(const size_t index)
	{
		return GetOrCreate<BedList, Bed>(beds, index, true);
	}

	Bed* GetFirstBed()
	{
		return Find<BedList, Bed>(beds, [](Bed* bed) { return bed->heater > -1; });
	}

	Bed* GetBedForHeater(const size_t heater)
	{
		return Find<BedList, Bed>(beds, [heater](Bed* bed) { return bed->heater == (int)heater; });
	}

	size_t GetBedCount()
	{
		return beds.Size();
	}

	void IterateBeds(stdext::inplace_function<void(Bed*)> func, const size_t startAt)
	{
		Iterate(beds, func, startAt);
	}

	Chamber* GetChamber(const size_t index)
	{
		return GetOrCreate<ChamberList, Chamber>(chambers, index, false);
	}

	Chamber* GetOrCreateChamber(const size_t index)
	{
		return GetOrCreate<ChamberList, Chamber>(chambers, index, true);
	}

	Chamber* GetFirstChamber()
	{
		return Find<ChamberList, Chamber>(chambers, [](Chamber* chamber) { return chamber->heater > -1; });
	}

	Chamber* GetChamberForHeater(const size_t heater)
	{
		return Find<ChamberList, Chamber>(chambers, [heater](Chamber* chamber) { return chamber->heater == (int)heater; });
	}

	size_t GetChamberCount()
	{
		return chambers.Size();
	}

	void IterateChambers(stdext::inplace_function<void(Chamber*)> func, const size_t startAt)
	{
		Iterate(chambers, func, startAt);
	}

	void GetHeaterSlots(
			const size_t heaterIndex,
			HeaterSlots& heaterSlots,
			const bool addTools,
			const bool addBeds,
			const bool addChambers)
	{
		if (addBeds)
		{
			IterateBeds(
				[&heaterSlots, heaterIndex](Bed* bed) {
					if (bed->slot < MaxSlots && bed->heater == (int)heaterIndex)
					{
						heaterSlots.Add(bed->slot);
					}
				});
		}
		if (addChambers)
		{
			IterateChambers(
				[&heaterSlots, heaterIndex](Chamber* chamber) {
					if (chamber->slot < MaxSlots && chamber->heater == (int)heaterIndex)
					{
						heaterSlots.Add(chamber->slot);
					}
				});
		}
		if (addTools)
		{
			IterateTools(
				[&heaterSlots, heaterIndex](Tool* tool) {
					if (tool->slot < MaxSlots && tool->heater == (int)heaterIndex)
					{
						heaterSlots.Add(tool->slot);
					}
				});
		}

	}

	size_t RemoveAxis(const size_t index, const bool allFollowing)
	{
		return Remove<AxisList, Axis>(axes, index, allFollowing);
	}

	size_t RemoveSpindle(const size_t index, const bool allFollowing)
	{
		return Remove<SpindleList, Spindle>(spindles, index, allFollowing);
	}

	size_t RemoveTool(const size_t index, const bool allFollowing)
	{
		return Remove<ToolList, Tool>(tools, index, allFollowing);
	}

	size_t RemoveBed(const size_t index, const bool allFollowing)
	{
		return Remove<BedList, Bed>(beds, index, allFollowing);
	}

	size_t RemoveChamber(const size_t index, const bool allFollowing)
	{
		return Remove<ChamberList, Chamber>(chambers, index, allFollowing);
	}
}



