/*
 * mem.cpp
 *
 * Created: 03/11/2014 14:14:17
 *  Author: David
 */ 

#include "ecv.h"
#include "Mem.hpp"

//#include <new>
//void* operator new(size_t objsize, std::nothrow_t dummy) {
//	return malloc(objsize);
//}

extern int  _end, __ram_end__;

static unsigned char *heap = NULL;

void* operator new(size_t objsize)
{
	if (heap == NULL)
	{
		heap = (unsigned char *)&_end;
	}
	
	void *prev_heap = heap;
	heap += objsize;
	return prev_heap;
}

void operator delete(void* obj) { }
	
unsigned int getFreeMemory()
{
	if (heap == NULL)
	{
		heap = (unsigned char *)&_end;
	}
	return (unsigned char *)&__ram_end__ - heap;	
}

// End
