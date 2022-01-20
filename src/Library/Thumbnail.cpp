#include "Library/Thumbnail.hpp"

#define DEBUG 2
#include "Debug.hpp"

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
