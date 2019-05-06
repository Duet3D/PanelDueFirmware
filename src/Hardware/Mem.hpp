/*
 * memh.h
 *
 * Created: 03/11/2014 14:18:15
 *  Author: David
 */ 


#ifndef MEM_H_
#define MEM_H_

#include <cstddef>
#include "chipid.h"

void* operator new(size_t objsize);

void operator delete(void* obj);

uint32_t GetRamEnd();
void InitMemory();
uint32_t GetRamSize();
uint32_t GetFreeMemory();

#endif /* MEMH_H_ */

// End
