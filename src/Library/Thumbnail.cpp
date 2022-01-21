#include "Library/Thumbnail.hpp"

extern "C"
{
#include "base64.h"
}

#include "sys/param.h"

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
	if (data.size == 0)
	{
		return false;
	}

	if (!data.buffer)
	{
		return false;
	}

	return true;
}

int ThumbnailDecodeChunk(struct Thumbnail &thumbnail, struct ThumbnailData &data, ThumbnailProcessCb callback, void *callbackContext)
{
	if (!ThumbnailIsValid(thumbnail))
	{
		dbg("meta invalid.\n");
		return -1;
	}

	if (!ThumbnailDataIsValid(data))
	{
		dbg("data invalid.\n");
		return -2;
	}

	dbg("size %d buffer %s\n", data.size, data.buffer);
	int ret = base64_decode(data.buffer, data.size, reinterpret_cast<unsigned char *>(data.buffer));
	if (ret < 0)
	{
		dbg("decode error %d size %d data\n%s\n",
			ret, data.size, data.buffer);
		return -3;
	}

	if (callback)
	{
		ret = callback(callbackContext, reinterpret_cast<unsigned char *>(data.buffer), ret);
		if (ret < 0)
		{
			return -4;
		}
	}

	return 0;
}
