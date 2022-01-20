#ifndef THUMBNAIL
#define THUMBNAIL 1

#include <cstdint>

struct ThumbnailData
{
	uint16_t size;
	uint16_t sizeTotal;
	uint16_t next;

	const char *buffer;
};

struct Thumbnail
{
	const char *filename;

	uint16_t width;
	uint16_t height;
	uint16_t offset;
	uint16_t size;

	enum ImageFormat {
		Invalid = 0,
		Qoi,
	} imageFormat;
};

#endif /* ifndef THUMBNAIL */
