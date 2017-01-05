/*
 * memh.h
 *
 * Created: 03/11/2014 14:18:15
 *  Author: David
 */ 


#ifndef MEM_H_
#define MEM_H_

#include <cstddef>

void* operator new(size_t objsize);

void operator delete(void* obj);

unsigned int getFreeMemory();

#endif /* MEMH_H_ */

// End
