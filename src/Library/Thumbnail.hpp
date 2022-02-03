#ifndef THUMBNAIL_HPP
#define THUMBNAIL_HPP 1

#include <cstdint>
#include <cstddef>

#include "qoi.h"


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
	unsigned char buffer[1024];
};

typedef void (*ThumbnailProcessCb)(const struct Thumbnail &thumbnail, uint32_t pixels_offset, const qoi_rgba_t *pixels, size_t pixels_count);

bool ThumbnailIsValid(struct Thumbnail &thumbnail);
bool ThumbnailDataIsValid(struct ThumbnailData &data);

int ThumbnailInit(struct Thumbnail &thumbnail);
int ThumbnailDecodeChunk(struct Thumbnail &thumbnail, struct ThumbnailData &data, ThumbnailProcessCb callback);

#endif /* ifndef THUMBNAIL_HPP */
