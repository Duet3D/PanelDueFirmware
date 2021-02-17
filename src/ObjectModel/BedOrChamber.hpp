/*
 * BedOrChamber.hpp
 *
 *  Created on: 17 Feb 2021
 *      Author: manuel
 */

#ifndef SRC_OBJECTMODEL_BEDORCHAMBER_HPP_
#define SRC_OBJECTMODEL_BEDORCHAMBER_HPP_


#include <cstdint>
#include "HeaterStatus.hpp"
#include <General/FreelistManager.h>
#include <General/inplace_function.h>

namespace OM
{
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

		void Reset();
	};

	typedef BedOrChamber Bed;
	typedef BedOrChamber Chamber;

	Bed* GetBed(const size_t index);
	Bed* GetOrCreateBed(const size_t index);
	Bed* GetFirstBed();
	size_t GetBedCount();
	void IterateBeds(stdext::inplace_function<void(Bed*)> func, const size_t startAt = 0);
	size_t RemoveBed(const size_t index, const bool allFollowing);

	Chamber* GetChamber(const size_t index);
	Chamber* GetOrCreateChamber(const size_t index);
	Chamber* GetFirstChamber();
	size_t GetChamberCount();
	void IterateChambers(stdext::inplace_function<void(Chamber*)> func, const size_t startAt = 0);
	size_t RemoveChamber(const size_t index, const bool allFollowing);
}

#endif /* SRC_OBJECTMODEL_BEDORCHAMBER_HPP_ */
