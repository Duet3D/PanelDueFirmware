/*
 * ObjectModel.hpp
 *
 *  Created on: 7 Sep 2020
 *      Author: manuel
 */

#ifndef SRC_OBJECTMODEL_OBJECTMODEL_HPP_
#define SRC_OBJECTMODEL_OBJECTMODEL_HPP_

#include <cstdint>
#include "Axis.hpp"
#include "BedOrChamber.hpp"
#include "Spindle.hpp"
#include "Tool.hpp"
#include "UserInterfaceConstants.hpp"
#include <General/Vector.hpp>

namespace OM {
	typedef Vector<uint8_t, MaxSlots> Slots;

	void GetHeaterSlots(
			const size_t heaterIndex,
			Slots& slots,
			const bool addTools = true,
			const bool addBeds = true,
			const bool addChambers = true);
}

#endif /* SRC_OBJECTMODEL_OBJECTMODEL_HPP_ */
