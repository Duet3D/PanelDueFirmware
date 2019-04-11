#include "FlashStorage.hpp"
#include <Library/Misc.hpp>
#include <chipid.h>
#include <cstring>

extern int __flash_start__,	__flash_end__;

extern void PrintDebugText(const char*);

//  FLASH_DEBUG can be enabled to get debugging information displayed.
#define FLASH_DEBUG(x) PrintDebugText(x)

static const uint32_t NVPSizes[] =
{
	0,
	8 * 1024,
	16 * 1024,
	32 * 1024,
	0,
	64 * 1024,
	0,
	128 * 1024,
	160 * 1024,
	256 * 1024,
	512 * 1024,
	0,
	1024 * 1024,
	0,
	2048 * 1024,
	0
};

static_assert(ARRAY_SIZE(NVPSizes) == 16, "invalid NVPSizes table");

static uint32_t GetFlashSize()
{
	return NVPSizes[chipid_read_nvpmsize(CHIPID)];
}

// Return the start address of the area we use to store non-volatile data.
// Version 2 boards use either a SAM3S2 or a SAM3S4 chip, so the address depends on which one is fitted.
static uint32_t GetNvDataStartAddress()
{
	return (uint32_t)(&__flash_start__) + GetFlashSize() - FLASH_DATA_LENGTH;
}

void FlashStorage::read(uint32_t address, void *data, uint32_t dataLength)
{
	memcpy(data, reinterpret_cast<const uint8_t*>(GetNvDataStartAddress()) + address, dataLength);
}

bool FlashStorage::write(uint32_t address, const void *data, uint32_t dataLength)
{
	const uint32_t nvStart = GetNvDataStartAddress();
	if (nvStart + address < (uint32_t)&__flash_start__)
	{
		FLASH_DEBUG("Flash write address too low");
		return false;		// write address too low
	}

	if (nvStart + address + dataLength > (uint32_t)&__flash_start__ + GetFlashSize())
	{
		FLASH_DEBUG("Flash write address too high");
		return false;		// write address too high
	}

	if (((nvStart + address) & 3) != 0)
	{
		FLASH_DEBUG("Flash start address must be on 4-byte boundary\n");
		return false;
	}

	// The flash management code in the ASF is fragile and has a tendency to fail to return. Help it by disabling interrupts.
#if SAM4S
	efc_disable_frdy_interrupt(EFC0);								// should not be enabled already, but disable it just in case
#else
	efc_disable_frdy_interrupt(EFC);								// should not be enabled already, but disable it just in case
#endif
	irqflags_t flags = cpu_irq_save();

	// Unlock page
	uint32_t retCode = flash_unlock(nvStart + address, (uint32_t)GetNvDataStartAddress() + address + dataLength - 1, NULL, NULL);
	if (retCode != FLASH_RC_OK)
	{
		FLASH_DEBUG("Failed to unlock flash for write");
	}
	else
	{
		// Write data
		retCode = flash_write(nvStart + address, data, dataLength, 1);
		if (retCode != FLASH_RC_OK)
		{
			FLASH_DEBUG("Flash write failed");
		}
		else
		{
			// Lock page
			retCode = flash_lock(nvStart + address, nvStart + address + dataLength - 1, NULL, NULL);
			if (retCode != FLASH_RC_OK)
			{
				FLASH_DEBUG("Failed to lock flash page");
			}
		}
	}

	cpu_irq_restore(flags);
	return retCode == FLASH_RC_OK;
}

// End
