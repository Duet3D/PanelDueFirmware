#include "Library/Thumbnail.hpp"

#define DEBUG 0
#include "Debug.hpp"

bool ThumbnailIsValid(struct Thumbnail &thumbnail)
{
	if (thumbnail.imageFormat != Thumbnail::ImageFormat::Qoi)
	{
		return false;
	}

	if (thumbnail.height == 0 || thumbnail.width == 0)
	{
		return false;
	}

	return true;
}

bool ThumbnailDataIsValid(struct ThumbnailData &data)
{
	if (!data.buffer)
	{
		return false;
	}

	return true;
}

int ThumbnailDecodeChunk(struct Thumbnail &thumbnail, struct ThumbnailData &data, ThumbnailProcessCb callback)
{
	dbg("format %s size %d data %s",
		thumbnail.imageFormat == Thumbnail::ImageFormat::Qoi ? "qoi" : "invalid",
		data.size,
		data.buffer);

	// TODO
	// do validity checks
	// decode base64
	// decode qoi to rgba
	// send data to callback

	return 0;
}
