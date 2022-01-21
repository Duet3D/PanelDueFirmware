#include "Library/Thumbnail.hpp"

#define DEBUG 2
#include "Debug.hpp"

bool ThumbnailIsValid(struct Thumbnail &thumbnail)
{
	if (thumbnail.imageFormat != Thumbnail::ImageFormat::Qoi)
	{
		return false;
	}

	if (thumbnail.size == 0)
	{
		return false;
	}

	if (thumbnail.height == 0 || thumbnail.width == 0)
	{
		return false;
	}

#if 0 // TODO remove this?
	if (!thumbnail.filename || strlen(thumbnail.filename) == 0)
	{
		return false;
	}
#endif

	return true;
}

bool ThumbnailDataIsValid(struct Thumbnail &thumbnail, struct ThumbnailData &data)
{
	if (!ThumbnailIsValid(thumbnail))
	{
		return false;
	}

	if (!data.buffer)
	{
		return false;
	}

	if (data.size >= thumbnail.size)
	{
		return false;
	}

	return true;
}

int ThumbnailDecodeChunk(struct Thumbnail &thumbnail, struct ThumbnailData &data, ThumbnailProcessCb callback)
{
	dbg("name %s format %s size %d data %s",
		thumbnail.filename,
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
