/*
 * ObjectModel.cpp
 *
 *  Created on: 7 Sep 2020
 *      Author: manuel
 */

#include <ObjectModel/Utils.hpp>
#include "Axis.hpp"
#include "BedOrChamber.hpp"
#include "Spindle.hpp"
#include "Tool.hpp"
#include "ListHelpers.hpp"
#include "PanelDue.hpp"

namespace OM
{
	void GetHeaterSlots(
			const size_t heaterIndex,
			Slots& slots,
			const bool addTools,
			const bool addBeds,
			const bool addChambers)
	{
		if (addBeds)
		{
			IterateBedsWhile(
				[&slots, heaterIndex](Bed*& bed, size_t) {
					if (bed->slot < MaxSlots && bed->heater == (int)heaterIndex)
					{
						slots.Add(bed->slot);
					}
					return bed->slot < MaxSlots;
				});
		}
		if (addChambers)
		{
			IterateChambersWhile(
				[&slots, heaterIndex](Chamber*& chamber, size_t) {
					if (chamber->slot < MaxSlots && chamber->heater == (int)heaterIndex)
					{
						slots.Add(chamber->slot);
					}
					return chamber->slot < MaxSlots;
				});
		}
		if (addTools)
		{
			IterateToolsWhile(
				[&slots, heaterIndex](Tool*& tool, size_t) {
					if (tool->slot < MaxSlots)
					{
						if (GetHeaterCombineType() == HeaterCombineType::notCombined)
						{
							tool->IterateHeaters([tool, &slots, heaterIndex](ToolHeater* th, size_t index) {
								if (tool->slot + index < MaxSlots && th->heaterIndex == (int) heaterIndex)
								{
									slots.Add(tool->slot + index);
								}
							});
						}
						else
						{
							if (tool->slot < MaxSlots && tool->heaters[0] != nullptr && tool->heaters[0]->heaterIndex == (int) heaterIndex)
							{
								slots.Add(tool->slot);
							}
						}
					}
					return tool->slot < MaxSlots;
				});
		}
	}
}
