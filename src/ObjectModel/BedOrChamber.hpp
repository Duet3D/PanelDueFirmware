/*
 * BedOrChamber.hpp
 *
 *  Created on: 17 Feb 2021
 *      Author: manuel
 */

#ifndef SRC_OBJECTMODEL_BEDORCHAMBER_HPP_
#define SRC_OBJECTMODEL_BEDORCHAMBER_HPP_


#include <cstdint>
#include <General/FreelistManager.h>
#include <General/function_ref.h>

namespace OM
{
	enum class HeaterStatus
	{
		off = 0,
		standby,
		active,
		fault,
		tuning,
		offline
	};


	struct HeaterStatusMapEntry
	{
		const char* key;
		const HeaterStatus val;
	};

	// This table has to be kept in alphabetical order of the keys
	const HeaterStatusMapEntry heaterStatusMap[] =
	{
		{ "active",		HeaterStatus::active },
		{ "fault",		HeaterStatus::fault },
		{ "off",	 	HeaterStatus::off },
		{ "offline",	HeaterStatus::offline },
		{ "standby",	HeaterStatus::standby },
		{ "tuning",		HeaterStatus::tuning },
	};

	struct BedOrChamber
	{
		void* operator new(size_t) noexcept { return FreelistManager::Allocate<BedOrChamber>(); }
		void operator delete(void* p) noexcept { FreelistManager::Release<BedOrChamber>(p); }

		// Index within configured heaters
		uint8_t index;
		// Id of heater
		int8_t heater;
		HeaterStatus heaterStatus;
		// Slot for display on panel
		uint8_t slot;
		uint8_t slotPJog;
		uint8_t slotPJob;

		void Reset();
	};

	typedef BedOrChamber Bed;
	typedef BedOrChamber Chamber;

	Bed* GetBed(const size_t index);
	Bed* GetOrCreateBed(const size_t index);
	Bed* GetFirstBed();
	size_t GetBedCount();
	bool IterateBedsWhile(function_ref<bool(Bed*&, size_t)> func, const size_t startAt = 0);
	size_t RemoveBed(const size_t index, const bool allFollowing);

	Chamber* GetChamber(const size_t index);
	Chamber* GetOrCreateChamber(const size_t index);
	Chamber* GetFirstChamber();
	size_t GetChamberCount();
	bool IterateChambersWhile(function_ref<bool(Chamber*&, size_t)> func, const size_t startAt = 0);
	size_t RemoveChamber(const size_t index, const bool allFollowing);
}

#endif /* SRC_OBJECTMODEL_BEDORCHAMBER_HPP_ */
