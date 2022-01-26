#include "Library/Thumbnail.hpp"

extern "C"
{
#include "base64.h"
}

#include "sys/param.h"

#define QOI_IMPLEMENTATION 1
#include "qoi.h"

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

int ThumbnailInit(struct Thumbnail &thumbnail)
{
	return qoi_decode_init(&thumbnail.qoi);
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

	int size_done = 0;
	int pixels_decoded = 0;
	unsigned char buffer[128];

	do
	{
		ret = qoi_decode_chunked(&thumbnail.qoi, ((const unsigned char *)data.buffer) + size_done, data.size - size_done, buffer, sizeof(buffer), &pixels_decoded);
		if (ret < 0)
		{
			return -4;
		}

		size_done += ret;
		thumbnail.pixel_count += pixels_decoded;

		//dbg("ret %d done %d/%d decoded %d/%d count %d\n",
			//ret, size_done, data.size, pixels_decoded, thumbnail.height * thumbnail.width, pixel_count);

		if (callback)
		{
//typedef int (*ThumbnailProcessCb)(void *context, const unsigned char *data, size_t size);
			callback(callbackContext, buffer, thumbnail.pixel_count * sizeof(qoi_rgba_t));
		}

	} while (thumbnail.pixel_count < thumbnail.qoi.width * thumbnail.qoi.height && size_done < data.size);

	dbg("done %d/%d pixels %d/%d\n",
		size_done, data.size, thumbnail.pixel_count, thumbnail.height * thumbnail.width);

	return 0;
}
