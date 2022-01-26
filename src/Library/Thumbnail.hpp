#ifndef THUMBNAIL_HPP
#define THUMBNAIL_HPP 1

#include <cstdint>
#include <cstddef>

extern "C"
{
	#include "qoi.h"
}

struct Thumbnail
{
	uint16_t width;
	uint16_t height;

	uint32_t pixel_count;

	enum ImageFormat {
		Invalid = 0,
		Qoi,
	} imageFormat;

	qoi_desc qoi;
};

struct ThumbnailData
{
	uint16_t size;
	char buffer[1024];
};

typedef int (*ThumbnailProcessCb)(void *context, const unsigned char *data, size_t size);

bool ThumbnailIsValid(struct Thumbnail &thumbnail);
bool ThumbnailDataIsValid(struct ThumbnailData &data);

int ThumbnailInit(struct Thumbnail &thumbnail);
int ThumbnailDecodeChunk(struct Thumbnail &thumbnail, struct ThumbnailData &data, ThumbnailProcessCb callback, void *callbackContext);

#endif /* ifndef THUMBNAIL_HPP */
