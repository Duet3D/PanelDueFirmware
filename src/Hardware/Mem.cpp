/*
 * mem.cpp
 *
 * Created: 03/11/2014 14:14:17
 *  Author: David
 */ 

#include "Mem.hpp"
#include "Library/Misc.hpp"

const uint32_t memPattern = 0xA5A5A5A5;

extern int _end;				// end of allocated data, always on a 4-byte boundary

static unsigned char *heap = nullptr;

void InitMemory()
{
	if (heap == nullptr)
	{
		heap = (unsigned char *)&_end;
	}

	register const uint32_t* stack_ptr asm ("sp");
	uint32_t *heapend = reinterpret_cast<uint32_t*>(heap);
	while (heapend + 16 < stack_ptr)
	{
		*heapend++ = memPattern;
	}
}

void* operator new(size_t objsize)
{
	if (heap == nullptr)
	{
		heap = (unsigned char *)&_end;
	}
	
	void *prev_heap = heap;
	heap += (objsize + 3) & (~3);
	return prev_heap;
}

void operator delete(void* obj) { (void)obj; }
void operator delete(void* obj, unsigned int) { (void)obj; }

static const uint32_t SramSizes[] =
{
	48 * 1024,
	192 * 1024,
	384 * 1024,
	6 * 1024,
	24 * 1024,
	4 * 1024,
	80 * 1024,
	160 * 1024,
	8 * 1024,
	16 * 1024,
	32 * 1024,
	64 * 1024,
	128 * 1024,
	256 * 1024,
	96 * 1024,
	512 * 1024
};

static_assert(ARRAY_SIZE(SramSizes) == 16, "invalid NVPSizes table");

uint32_t GetRamSize()
{
	return SramSizes[chipid_read_sramsize(CHIPID)];
}

uint32_t GetFreeMemory()
{
	register const uint32_t * stack_ptr asm ("sp");
	const uint32_t *heapend = reinterpret_cast<const uint32_t*>(heap);
	while (heapend < stack_ptr && *heapend == memPattern)
	{
		++heapend;
	}

	return (unsigned char*)heapend - heap;
}

// End
