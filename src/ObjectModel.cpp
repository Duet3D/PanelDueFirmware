/*
 * ObjectModel.cpp
 *
 *  Created on: 7 Sep 2020
 *      Author: manuel
 */

#include "ObjectModel.hpp"

template<class T>
T* GetOrCreate(T*& start, size_t index, bool create)
{
	T* ret = start;
	for (; ret != nullptr; ret = ret->next)
	{
		if (ret->index == index)
		{
			break;
		}
	}

	// Not found, create new
	if (ret == nullptr && create)
	{
		ret = new T;
		ret->index = index;
		if (start == nullptr || start->index > index)
		{
			ret->next = start;
			start = ret;
		}
		else
		{
			// Insert it at the correct spot
			auto current = start;
			while (current->next != nullptr && current->next->index < index)
			{
				current = current->next;
			}
			ret->next = current->next;
			current->next = ret;
		}
	}
	return ret;
}

template<class T>
size_t GetElementCount(T*& start)
{
	size_t count = 0;
	for (auto elem = start; elem != nullptr; elem = elem->next)
	{
		++count;
	}
	return count;
}

template<typename T>
T* Find(T*& start, std::function<bool(T*)> filter)
{
	for (auto elem = start; elem != nullptr; elem = elem->next)
	{
		if (filter(elem))
		{
			return elem;
		}
	}
	return nullptr;
}

template<typename T>
void Iterate(T*& start, std::function<void(T*)> func, const size_t startAt)
{
	size_t counter = 0;
	for (auto elem = start; elem != nullptr; elem = elem->next)
	{
		if (counter >= startAt)
		{
			func(elem);
		}
		++counter;
	}
}

template<typename T>
bool IterateWhile(T*& start, std::function<bool(T*)> func, const size_t startAt)
{
	size_t counter = 0;
	for (auto elem = start; elem != nullptr; elem = elem->next)
	{
		if (counter >= startAt)
		{
			if(!func(elem))
			{
				return false;
			}
		}
		++counter;
	}
	return true;
}

template<typename T>
size_t Remove(T*& start, size_t index, bool allFollowing)
{
	size_t removed = 0;
	// Nothing to do on an empty list
	if (start == nullptr)
	{
		return removed;
	}

	if (!allFollowing)	// We are looking for an exact match
	{
		if (start->index == index)
		{
			auto toDelete = start;
			start = start->next;
			delete toDelete;
			++removed;
		}
		else
		{
			T* toDelete = start;
			T* prev;
			do
			{
				prev = toDelete;
				toDelete = toDelete->next;
			} while (toDelete != nullptr && toDelete->index != index);

			if (toDelete != nullptr)
			{
				prev->next = toDelete->next;
				delete toDelete;
				++removed;
			}
		}
	}
	else	// we want to delete eveything with elem->index >= index
	{
		// We need to delete the full list
		if (start->index >= index)
		{
			while (start != nullptr)
			{
				auto toDelete = start;
				start = start->next;
				delete toDelete;
				++removed;
			}
		}
		else
		{
			for (auto s = start; s != nullptr; s = s->next)
			{
				if (s->next == nullptr || s->next->index < index)
				{
					continue;
				}
				// If we get here then s->next->index >= index
				for (auto toDelete = s->next; toDelete != nullptr; toDelete = s->next)
				{
					s->next = toDelete->next;
					delete toDelete;
					++removed;
				}
			}
		}
	}
	return removed;
}

namespace OM
{
	static Axis* axes;
	static Spindle* spindles;
	static Tool* tools;
	static Bed* beds;
	static Chamber* chambers;

	Axis* FindAxis(std::function<bool(Axis*)> filter)
	{
		return Find(axes, filter);
	}

	Axis* GetAxis(const size_t index)
	{
		if (index >= MaxTotalAxes)
		{
			return nullptr;
		}
		return GetOrCreate(axes, index, false);
	}

	Axis* GetAxisInSlot(const size_t slot)
	{
		if (slot >= MaxTotalAxes)
		{
			return nullptr;
		}
		return Find<Axis>(axes, [slot](auto axis) { return axis->slot == slot; });
	}

	Axis* GetOrCreateAxis(const size_t index)
	{
		if (index >= MaxTotalAxes)
		{
			return nullptr;
		}
		return GetOrCreate(axes, index, true);
	}

	void IterateAxes(std::function<void(Axis*)> func, const size_t startAt)
	{
		Iterate(axes, func, startAt);
	}

	bool IterateAxesWhile(std::function<bool(Axis*)> func, const size_t startAt)
	{
		return IterateWhile(axes, func, startAt);
	}

	Spindle* GetSpindle(const size_t index)
	{
		return GetOrCreate(spindles, index, false);
	}

	Spindle* GetOrCreateSpindle(const size_t index)
	{
		return GetOrCreate(spindles, index, true);
	}

	void IterateSpindles(std::function<void(Spindle*)> func, const size_t startAt)
	{
		Iterate(spindles, func, startAt);
	}

	bool IterateSpindlesWhile(std::function<bool(Spindle*)> func, const size_t startAt)
	{
		return IterateWhile(spindles, func, startAt);
	}

	Tool* GetTool(const size_t index)
	{
		return GetOrCreate(tools, index, false);
	}

	Tool* GetOrCreateTool(const size_t index)
	{
		return GetOrCreate(tools, index, true);
	}

	void IterateTools(std::function<void(Tool*)> func, const size_t startAt)
	{
		Iterate(tools, func, startAt);
	}

	bool IterateToolsWhile(std::function<bool(Tool*)> func, const size_t startAt)
	{
		return IterateWhile(tools, func, startAt);
	}

	Spindle* GetSpindleForTool(const size_t toolNumber)
	{
		return Find<Spindle>(spindles, [toolNumber](auto spindle) { return spindle->tool == (int)toolNumber; });
	}

	Tool* GetToolForExtruder(const size_t extruder)
	{
		return Find<Tool>(tools, [extruder](auto tool) { return tool->extruder == (int)extruder; });
	}

	Tool* GetToolForHeater(const size_t heater)
	{
		return Find<Tool>(tools, [heater](auto tool) { return tool->heater == (int)heater; });
	}

	Bed* GetBed(const size_t index)
	{
		return GetOrCreate(beds, index, false);
	}

	Bed* GetOrCreateBed(const size_t index)
	{
		return GetOrCreate(beds, index, true);
	}

	Bed* GetFirstBed()
	{
		return Find<Bed>(beds, [](auto bed) { return bed->heater > -1; });
	}

	Bed* GetBedForHeater(const size_t heater)
	{
		return Find<Bed>(beds, [heater](auto bed) { return bed->heater == (int)heater; });
	}

	size_t GetBedCount()
	{
		return GetElementCount(beds);
	}

	void IterateBeds(std::function<void(Bed*)> func, const size_t startAt)
	{
		Iterate(beds, func, startAt);
	}

	Chamber* GetChamber(const size_t index)
	{
		return GetOrCreate(chambers, index, false);
	}

	Chamber* GetOrCreateChamber(const size_t index)
	{
		return GetOrCreate(chambers, index, true);
	}

	Chamber* GetFirstChamber()
	{
		return Find<Chamber>(chambers, [](auto chamber) { return chamber->heater > -1; });
	}

	Chamber* GetChamberForHeater(const size_t heater)
	{
		return Find<Chamber>(chambers, [heater](auto chamber) { return chamber->heater == (int)heater; });
	}

	size_t GetChamberCount()
	{
		return GetElementCount(chambers);
	}

	void IterateChambers(std::function<void(Chamber*)> func, const size_t startAt)
	{
		Iterate(chambers, func, startAt);
	}

	size_t RemoveAxis(const size_t index, const bool allFollowing)
	{
		return Remove(axes, index, allFollowing);
	}

	size_t RemoveSpindle(const size_t index, const bool allFollowing)
	{
		return Remove(spindles, index, allFollowing);
	}

	size_t RemoveTool(const size_t index, const bool allFollowing)
	{
		return Remove(tools, index, allFollowing);
	}

	size_t RemoveBed(const size_t index, const bool allFollowing)
	{
		return Remove(beds, index, allFollowing);
	}

	size_t RemoveChamber(const size_t index, const bool allFollowing)
	{
		return Remove(chambers, index, allFollowing);
	}
}



