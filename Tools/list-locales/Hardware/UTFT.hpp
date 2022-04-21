/*
 * Dummy header to avoid unnecessary includes
*/

#ifndef UTFT_h
#define UTFT_h

#include <cstdint>

typedef uint16_t Colour;

const Colour black = 0x0000;
const Colour white = 0xFFFF;

typedef const uint16_t *Palette;

namespace UTFT {
inline constexpr uint16_t fromRGB(uint8_t r, uint8_t g, uint8_t b)
{
	return ((r & 248) << 8) | ((g & 252) << 3) | (b >> 3);
}
}

#endif
