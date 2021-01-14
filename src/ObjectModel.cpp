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
T* GetOrCreate(L& list, const size_t index, const bool create)
{
	const size_t count = list.Size();
	for (size_t i = 0; i < count; ++i)
	{
		if (list[i]->index == index)
		{
			return list[i];
		}
	}

	if (create && !list.Full())
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
size_t Remove(L& list, const size_t index, const bool allFollowing)
{
	// Nothing to do on an empty list or
	// if the last element is already smaller than what we look for
	if (list.IsEmpty() || list[list.Size()-1]->index < index)
	{
		return 0;
	}

	size_t removed = 0;
	for (size_t i = list.Size(); i != 0;)
	{
		--i;
		T* elem = list[i];
		if (elem->index == index || (allFollowing && elem->index > index))
		{
			list.Erase(i);
			delete elem;
			++removed;
			if (!allFollowing)
			{
				break;
			}
		}
	}
	return removed;
}

void OM::Axis::Reset()
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

void OM::Spindle::Reset()
{
	index = 0;
	active = 0;
	max = 10000;
	min = 0;
	tool = -1;
}

void OM::Tool::SetHeater(const uint8_t toolHeaterIndex, const uint8_t heaterIndex)
{
	if (toolHeaterIndex >= MaxSlots)
	{
		return;
	}
	heaters[toolHeaterIndex] = heaterIndex;
}

int8_t OM::Tool::HasHeater(const uint8_t heaterIndex) const
{
	for (size_t i = 0; i < MaxSlots; ++i)
	{
		if (heaters[i] == (int) heaterIndex)
		{
			return i;
		}
	}
	return -1;
}

size_t OM::Tool::RemoveHeatersFrom(const uint8_t toolHeaterIndex)
{
	size_t removed = 0;
	for (size_t i = toolHeaterIndex; i < MaxSlots; ++i)
	{
		heaters[i] = -1;
		++removed;
	}
	return removed;
}

void OM::Tool::IterateHeaters(stdext::inplace_function<void(uint8_t)> func, const size_t startAt)
{
	for (size_t i = startAt; i < MaxSlots && heaters[i] > -1; ++i)
	{
		func((uint8_t)heaters[i]);
	}
}

void OM::Tool::Reset()
{
	index = 0;
	RemoveHeatersFrom(0);
	extruder = -1;
	spindle = nullptr;
	fan = -1;
	for (size_t i = 0; i < MaxTotalAxes; ++i)
	{
		offsets[i] = 0.0f;
	}
	status = ToolStatus::off;
	slot = MaxSlots;
}

void OM::BedOrChamber::Reset()
{
	index = 0;
	heater = -1;
	heaterStatus = HeaterStatus::off;
	slot = MaxSlots;
}

namespace OM
{
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

	Spindle* GetSpindle(const size_t index)
	{
		return GetOrCreate<SpindleList, Spindle>(spindles, index, false);
	}

	Spindle* GetOrCreateSpindle(const size_t index)
	{
		return GetOrCreate<SpindleList, Spindle>(spindles, index, true);
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

	Tool* GetToolForExtruder(const size_t extruder)
	{
		return Find<ToolList, Tool>(tools, [extruder](Tool* tool) { return tool->extruder == (int)extruder; });
	}

	Tool* GetToolForFan(const size_t fan)
	{
		return Find<ToolList, Tool>(tools, [fan](Tool* tool) { return tool->fan == (int)fan; });
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
					if (tool->slot < MaxSlots)
					{
						const int8_t heaterSubSlot = tool->HasHeater(heaterIndex);
						if (heaterSubSlot > -1)
						{
							heaterSlots.Add(tool->slot + heaterSubSlot);
						}
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
