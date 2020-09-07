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

template<typename T>
void Iterate(T*& start, std::function<void(T*)> func)
{
	for (auto elem = start; elem != nullptr; elem = elem->next)
	{
		func(elem);
	}
}

template<typename T>
bool IterateWhile(T*& start, std::function<bool(T*)> func)
{
	for (auto elem = start; elem != nullptr; elem = elem->next)
	{
		if(!func(elem))
		{
			return false;
		}
	}
	return true;
}

template<typename T>
void Remove(T*& start, size_t index, bool allFollowing)
{
	// TODO: Unify if-else-block
	if (start != nullptr && start->index >= index)
	{
		if (allFollowing)
		{
			for (auto toDelete = start; toDelete != nullptr; toDelete = start->next)
			{
				start = toDelete->next;
				delete toDelete;
			}
		}
		else
		{
			auto toDelete = start;
			start = start->next;
			delete toDelete;
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
			if (allFollowing)
			{
				for (auto toDelete = s->next; toDelete != nullptr; toDelete = s->next)
				{
					s->next = toDelete->next;
					delete toDelete;
				}
			}
			else
			{
				auto toDelete = s->next;
				s->next = s->next->next;
				delete toDelete;
			}
		}
	}
}

namespace OM
{
	static Axis* axes;
	static Spindle* spindles;
	static Tool* tools;

	Axis* GetAxis(size_t index)
	{
		if (index >= MaxTotalAxes)
		{
			return nullptr;
		}
		return GetOrCreate(axes, index, false);
	}

	Axis* GetOrCreateAxis(size_t index)
	{
		if (index >= MaxTotalAxes)
		{
			return nullptr;
		}
		return GetOrCreate(axes, index, true);
	}

	void IterateAxes(std::function<void(Axis*)> func)
	{
		Iterate(axes, func);
	}

	bool IterateAxesWhile(std::function<bool(Axis*)> func)
	{
		return IterateWhile(axes, func);
	}

	Spindle* GetSpindle(size_t index)
	{
		return GetOrCreate(spindles, index, false);
	}

	Spindle* GetOrCreateSpindle(size_t index)
	{
		return GetOrCreate(spindles, index, true);
	}

	void IterateSpindles(std::function<void(Spindle*)> func)
	{
		Iterate(spindles, func);
	}

	bool IterateSpindlesWhile(std::function<bool(Spindle*)> func)
	{
		return IterateWhile(spindles, func);
	}

	Tool* GetTool(size_t index)
	{
		return GetOrCreate(tools, index, false);
	}

	Tool* GetOrCreateTool(size_t index)
	{
		return GetOrCreate(tools, index, true);
	}

	void IterateTools(std::function<void(Tool*)> func)
	{
		Iterate(tools, func);
	}

	bool IterateToolsWhile(std::function<bool(Tool*)> func)
	{
		return IterateWhile(tools, func);
	}

	void RemoveAxis(size_t index, bool allFollowing)
	{
		Remove(axes, index, allFollowing);
	}

	void RemoveSpindle(size_t index, bool allFollowing)
	{
		Remove(spindles, index, allFollowing);
	}

	void RemoveTool(size_t index, bool allFollowing)
	{
		Remove(tools, index, allFollowing);
	}

	Spindle* GetSpindleForTool(size_t toolNumber)
	{
		for (auto spindle = spindles; spindle != nullptr; spindle = spindle->next)
		{
			if (spindle->tool == (int)toolNumber)
			{
				return spindle;
			}
		}
		return nullptr;
	}

	Tool* GetToolForExtruder(size_t extruder)
	{
		for (auto tool = tools; tool != nullptr; tool = tool->next)
		{
			if (tool->extruder == (int)extruder)
			{
				return tool;
			}
		}
		return nullptr;
	}

	Tool* GetToolForHeater(size_t heater)
	{
		for (auto tool = tools; tool != nullptr; tool = tool->next)
		{
			if (tool->heater == (int)heater)
			{
				return tool;
			}
		}
		return nullptr;
	}
}



