#ifndef THUMBNAIL_HPP
#define THUMBNAIL_HPP 1

#include <cstdint>
#include <cstddef>

struct Thumbnail
{
	uint16_t width;
	uint16_t height;

	enum ImageFormat {
		Invalid = 0,
		Qoi,
	} imageFormat;
};

struct ThumbnailData
{
	uint16_t size;
	const char *buffer;
};

typedef int (*ThumbnailProcessCb)(const char *data, size_t size);

bool ThumbnailIsValid(struct Thumbnail &thumbnail);
bool ThumbnailDataIsValid(struct ThumbnailData &data);

int ThumbnailDecodeChunk(struct Thumbnail &thumbnail, struct ThumbnailData &data, ThumbnailProcessCb callback);

#endif /* ifndef THUMBNAIL_HPP */
