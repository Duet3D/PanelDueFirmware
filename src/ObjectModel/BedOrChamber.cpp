/*
 * BedOrChamber.cpp
 *
 *  Created on: 17 Feb 2021
 *      Author: manuel
 */

#include "BedOrChamber.hpp"
#include "ListHelpers.hpp"
#include <General/Vector.hpp>
#include <UI/UserInterfaceConstants.hpp>

typedef Vector<OM::Bed*, MaxSlots> BedList;
typedef Vector<OM::Chamber*, MaxSlots> ChamberList;

static BedList beds;
static ChamberList chambers;

namespace OM
{
	void BedOrChamber::Reset()
	{
		index = 0;
		heater = -1;
		heaterStatus = HeaterStatus::off;
		slot = MaxSlots;
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

	size_t GetBedCount()
	{
		return beds.Size();
	}

	bool IterateBedsWhile(stdext::inplace_function<bool(Bed*&, size_t)> func, const size_t startAt)
	{
		return beds.IterateWhile(func, startAt);
	}

	size_t RemoveBed(const size_t index, const bool allFollowing)
	{
		return Remove<BedList, Bed>(beds, index, allFollowing);
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

	size_t GetChamberCount()
	{
		return chambers.Size();
	}

	bool IterateChambersWhile(stdext::inplace_function<bool(Chamber*&, size_t)> func, const size_t startAt)
	{
		return chambers.IterateWhile(func, startAt);
	}

	size_t RemoveChamber(const size_t index, const bool allFollowing)
	{
		return Remove<ChamberList, Chamber>(chambers, index, allFollowing);
	}

}
