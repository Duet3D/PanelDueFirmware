/*
 * Tool.cpp
 *
 *  Created on: 17 Feb 2021
 *      Author: manuel
 */

#include "Tool.hpp"
#include "ListHelpers.hpp"
#include <General/Vector.hpp>
#include <UI/UserInterfaceConstants.hpp>

typedef Vector<OM::Tool*, MaxSlots> ToolList;
static ToolList tools;

namespace OM
{
	void ToolHeater::Reset()
	{
		heaterIndex = 0;
		activeTemp = 0;
		standbyTemp = 0;
	}

	void Tool::operator delete(void * p)
	{
		Tool* t = static_cast<Tool*>(p);
		for (size_t i = 0; i < MaxHeatersPerTool; ++i)
		{
			delete t->heaters[i];
		}
		FreelistManager::Release<Tool>(p);
	}

	ToolHeater* Tool::GetOrCreateHeater(const uint8_t toolHeaterIndex)
	{
		if (toolHeaterIndex >= MaxHeatersPerTool)
		{
			return nullptr;
		}
		if (heaters[toolHeaterIndex] != nullptr)
		{
			return heaters[toolHeaterIndex];
		}
		ToolHeater* th = new ToolHeater;
		th->Reset();
		heaters[toolHeaterIndex] = th;
		return th;
	}

	bool Tool::GetHeaterTemps(const StringRef& ref, const bool active)
	{
		for (size_t i = 0; i < MaxHeatersPerTool && heaters[i] != nullptr; ++i)
		{
			if (i > 0)
			{
				ref.cat(':');
			}
			ref.catf("%d", (active ? heaters[i]->activeTemp : heaters[i]->standbyTemp));
		}

		return !ref.IsEmpty();
	}

	int8_t Tool::HasHeater(const uint8_t heaterIndex) const
	{
		for (size_t i = 0; i < MaxHeatersPerTool && heaters[i] != nullptr; ++i)
		{
			if (heaters[i]->heaterIndex == (int) heaterIndex)
			{
				return i;
			}
		}
		return -1;
	}

	void Tool::IterateHeaters(function_ref<void(ToolHeater*, size_t)> func, const size_t startAt)
	{
		for (size_t i = startAt; i < MaxHeatersPerTool && heaters[i] != nullptr; ++i)
		{
			func(heaters[i], i);
		}
	}

	size_t Tool::RemoveHeatersFrom(const uint8_t heaterIndex)
	{
		if (heaterIndex >= MaxHeatersPerTool)
		{
			return 0;
		}
		size_t removed = 0;
		for (size_t i = heaterIndex; i < MaxHeatersPerTool && heaters[i] != nullptr; ++i)
		{
			delete heaters[i];
			heaters[i] = nullptr;
			++removed;
		}
		return removed;
	}

	void Tool::UpdateTemp(const uint8_t toolHeaterIndex, const int32_t temp, const bool active)
	{
		ToolHeater* toolHeater = GetOrCreateHeater(toolHeaterIndex);
		if (toolHeater == nullptr)
		{
			return;
		}
		if (active)
		{
			toolHeater->activeTemp = temp;
		}
		else
		{
			toolHeater->standbyTemp = temp;
		}
	}

	void Tool::Reset()
	{
		index = 0;
		for (size_t i = 0; i < MaxHeatersPerTool; ++i)
		{
			heaters[i] = nullptr;
		}
		extruders.Clear();
		spindle = nullptr;
		spindleRpm = 0;
		fans.Clear();
		for (size_t i = 0; i < MaxTotalAxes; ++i)
		{
			offsets[i] = 0.0f;
		}
		status = ToolStatus::off;
		slot = MaxSlots;
		slotPJog = MaxPendantTools;
		slotPJob = MaxPendantTools;
	}

	Tool* GetTool(const size_t index)
	{
		return GetOrCreate<ToolList, Tool>(tools, index, false);
	}

	Tool* GetOrCreateTool(const size_t index)
	{
		return GetOrCreate<ToolList, Tool>(tools, index, true);
	}

	bool IterateToolsWhile(function_ref<bool(Tool*&, size_t)> func, const size_t startAt)
	{
		return tools.IterateWhile(func, startAt);
	}

	size_t RemoveTool(const size_t index, const bool allFollowing)
	{
		return Remove<ToolList, Tool>(tools, index, allFollowing);
	}
}
