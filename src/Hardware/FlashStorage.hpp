/* 
DueFlashStorage saves non-volatile data for Arduino Due.
The library is made to be similar to EEPROM library
Uses flash block 1 per default.

Note: uploading new software will erase all flash so data written to flash
using this library will not survive a new software upload. 

Inspiration from Pansenti at https://github.com/Pansenti/DueFlash
Rewritten and modified by Sebastian Nilsson
Further modified up by David Crocker
*/


#ifndef FLASHSTORAGE_H
#define FLASHSTORAGE_H

#include "asf.h"

#define FLASH_DATA_LENGTH   (64)			// 64 bytes of storage

extern int __flash_start__,	__flash_end__;

extern void PrintDebugText(const char*);

// Choose a start address close to the top of the Flash memory space
#define  FLASH_START  (((uint8_t *)&__flash_end__) - FLASH_DATA_LENGTH)
//#define FLASH_START		((uint8_t *)(IFLASH_ADDR + IFLASH_SIZE - FLASH_DATA_LENGTH))

//  FLASH_DEBUG can be enabled to get debugging information displayed.
#define FLASH_DEBUG(x) PrintDebugText(x)
//#define FLASH_DEBUG(x)


//  FlashStorage is the main namespace for flash functions
namespace FlashStorage
{
	// address is the offset into the flash storage area where we want to write the data
	// data is a pointer to the data to be written
	// dataLength is length of data in bytes
  
	void read(uint32_t address, void *data, uint32_t dataLength);
	bool write(uint32_t address, const void *data, uint32_t dataLength);
};

#endif
