/*
 * ObjectModel.cpp
 *
 *  Created on: 7 Sep 2020
 *      Author: manuel
 */

#include "ObjectModel.hpp"
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
			IterateBeds(
				[&slots, &heaterIndex](Bed* bed) {
					if (bed->slot < MaxSlots && bed->heater == (int)heaterIndex)
					{
						slots.Add(bed->slot);
					}
				});
		}
		if (addChambers)
		{
			IterateChambers(
				[&slots, &heaterIndex](Chamber* chamber) {
					if (chamber->slot < MaxSlots && chamber->heater == (int)heaterIndex)
					{
						slots.Add(chamber->slot);
					}
				});
		}
		if (addTools)
		{
			IterateTools(
				[&slots, &heaterIndex](Tool* tool) {
					if (tool->slot < MaxSlots)
					{
						if (GetHeaterCombineType() == HeaterCombineType::notCombined)
						{
							tool->IterateHeaters([&tool, &slots, &heaterIndex](ToolHeater* th, size_t index) {
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
				});
		}
	}
}
