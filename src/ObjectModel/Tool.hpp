/*
 * Tool.hpp
 *
 *  Created on: 17 Feb 2021
 *      Author: manuel
 */

#ifndef SRC_OBJECTMODEL_TOOL_HPP_
#define SRC_OBJECTMODEL_TOOL_HPP_

#include <cstdint>
#include "Spindle.hpp"
#include "ToolStatus.hpp"
#include "UserInterfaceConstants.hpp"
#include <General/FreelistManager.h>
#include <General/StringRef.h>
#include <General/inplace_function.h>

namespace OM
{
	struct ToolHeater
	{
		void* operator new(size_t) noexcept { return FreelistManager::Allocate<ToolHeater>(); }
		void operator delete(void* p) noexcept { FreelistManager::Release<ToolHeater>(p); }

		uint8_t heaterIndex;	// This is the heater number
		int16_t activeTemp;
		int16_t standbyTemp;

		void Reset();
	};

	struct Tool
	{
		void* operator new(size_t) noexcept { return FreelistManager::Allocate<Tool>(); }
		void operator delete(void* p) noexcept;

		// tool number
		uint8_t index;
		ToolHeater* heaters[MaxHeatersPerTool];
		int8_t extruder;			// only look at the first extruder as we only display one
		Spindle* spindle;
		int32_t spindleRpm;
		int8_t fan;
		float offsets[MaxTotalAxes];
		ToolStatus status;
		uint8_t slot;

		ToolHeater* GetOrCreateHeater(const uint8_t toolHeaterIndex);
		bool GetHeaterTemps(const StringRef& ref, const bool active);
		int8_t HasHeater(const uint8_t heaterIndex) const;
		void IterateHeaters(stdext::inplace_function<void(ToolHeater*, size_t)> func, const size_t startAt = 0);
		size_t RemoveHeatersFrom(const uint8_t toolHeaterIndex);
		void UpdateTemp(const uint8_t toolHeaterIndex, const int32_t temp, const bool active);

		void Reset();
	};

	Tool* GetTool(const size_t index);
	Tool* GetOrCreateTool(const size_t index);
	void IterateTools(stdext::inplace_function<void(Tool*)> func, const size_t startAt = 0);
	size_t RemoveTool(const size_t index, const bool allFollowing);
}

#endif /* SRC_OBJECTMODEL_TOOL_HPP_ */
