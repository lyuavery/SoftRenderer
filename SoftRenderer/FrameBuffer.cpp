#include "FrameBuffer.h"

#include "Math/Utility.h"

void SR::FrameBufferAttachment::Set(int x, int y, float r, float g, float b, float a)
{
	switch (format)
	{
	case SR::FrameBufferAttachmentFormat::Depth32: {
		if (x >= width || x < 0 || y < 0 || y >= height) return;
		auto offset = (x + y * width) * bytespp;
		auto ptr = reinterpret_cast<Byte*>(buffer);

		*((float*)(ptr + offset)) = r;
		break;
	}
	case SR::FrameBufferAttachmentFormat::BGR24: 
	case SR::FrameBufferAttachmentFormat::BGRA32:
	case SR::FrameBufferAttachmentFormat::ARGB32: {
		Set(x, y, Byte(sbm::clamp01(r) * 255),
			Byte(sbm::clamp01(g) * 255),
			Byte(sbm::clamp01(b) * 255),
			Byte(sbm::clamp01(a) * 255));
		break;
	}
	}
}

void SR::FrameBufferAttachment::Set(int x, int y, Byte r, Byte g, Byte b, Byte a)
{
	if (x >= width || x < 0 || y < 0 || y >= height) return;
	auto offset = (x + y * width) * bytespp;
	auto ptr = reinterpret_cast<Byte*>(buffer);
	switch (format)
	{
	case SR::FrameBufferAttachmentFormat::BGR24: {
		*(ptr + offset) = b;
		*(ptr + offset + 1) = g;
		*(ptr + offset + 2) = r;
		break;
	}
	case SR::FrameBufferAttachmentFormat::BGRA32: {
		*(ptr + offset) = b;
		*(ptr + offset + 1) = g;
		*(ptr + offset + 2) = r;
		*(ptr + offset + 3) = a;
		break;
	}
	case SR::FrameBufferAttachmentFormat::ARGB32: {
		*(ptr + offset) = a;
		*(ptr + offset + 1) = r;
		*(ptr + offset + 2) = g;
		*(ptr + offset + 3) = b;
		break;
	}
	case SR::FrameBufferAttachmentFormat::Depth32: {
		float denom = 1.f / 255.f;
		Set(x, y, r * denom, g * denom, b * denom, a * denom);
		break;
	}
	}
}

int SR::FrameBufferAttachment::Get(int x, int y, float& r, float& g, float& b, float& a) const
{
	int components = 0;
	
	switch (format)
	{
	case SR::FrameBufferAttachmentFormat::BGR24: 
	case SR::FrameBufferAttachmentFormat::BGRA32:
	case SR::FrameBufferAttachmentFormat::ARGB32: {
		Byte _r, _g, _b, _a;
		components = Get(x, y, _r, _g, _b, _a);
		r = _r, g = _g, b = _b, a = _a;
		break;
	}
	case SR::FrameBufferAttachmentFormat::Depth32: {
		if (x >= width || x < 0 || y < 0 || y >= height) return -1;
		auto ptr = reinterpret_cast<Byte*>(buffer);
		auto offset = (x + y * width) * bytespp;

		components = 1;
		r = *((float*)(ptr + offset));
		break;
	}
	}
	return components;
}

int SR::FrameBufferAttachment::Get(int x, int y, Byte& r, Byte& g, Byte& b, Byte& a) const
{
	if (x >= width || x < 0 || y < 0 || y >= height) return -1;
	int components = 0;
	auto ptr = reinterpret_cast<Byte*>(buffer);
	auto offset = (x + y * width) * bytespp;
	switch (format)
	{
	case SR::FrameBufferAttachmentFormat::BGR24: {
		components = 3;
		b = *(ptr + offset);
		g = *(ptr + offset + 1);
		r = *(ptr + offset + 2);
		break;
	}
	case SR::FrameBufferAttachmentFormat::BGRA32: {
		components = 4;
		b = *(ptr + offset);
		g = *(ptr + offset + 1);
		r = *(ptr + offset + 2);
		a = *(ptr + offset + 3);
		break;
	}
	case SR::FrameBufferAttachmentFormat::ARGB32: {
		components = 4;
		a = *(ptr + offset);
		r = *(ptr + offset + 1);
		g = *(ptr + offset + 2);
		b = *(ptr + offset + 3);
		break;
	}
	case SR::FrameBufferAttachmentFormat::Depth32: {
		float _r, _g, _b, _a;
		components = Get(x, y, _r, _g, _b, _a);
		r = _r, g = _g, b = _b, a = _a;
		break;
	}
	}
	return components;
}

void SR::FrameBufferAttachment::Clear(float r, float g, float b, float a)
{
	if (!buffer) return;
	for (int j = 0; j < height; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			Set(i, j, r, g, b, a);
		}
	}
}

void SR::FrameBufferAttachment::Clear(Byte r, Byte g, Byte b, Byte a)
{
	if (!buffer) return;
	for (int j = 0; j < height; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			Set(i, j, r, g, b, a);
		}
	}
}

int SR::FrameBufferAttachment::GetFormatBytesPerPixel(SR::FrameBufferAttachmentFormat fmt)
{
	int n = GetFormatChannels(fmt);
	int elemSize = 0;
	switch (fmt)
	{
	case SR::FrameBufferAttachmentFormat::BGRA32:
	case SR::FrameBufferAttachmentFormat::ARGB32:
	case SR::FrameBufferAttachmentFormat::BGR24:
	{
		elemSize = sizeof(Byte);
		break;
	}
	case SR::FrameBufferAttachmentFormat::Depth32:
	{
		elemSize = sizeof(float);
		break;
	}
	}
	return n * elemSize;
}

int SR::FrameBufferAttachment::GetFormatChannels(SR::FrameBufferAttachmentFormat fmt)
{
	int n = 0;
	switch (fmt)
	{
	case SR::FrameBufferAttachmentFormat::ARGB32:
	case SR::FrameBufferAttachmentFormat::BGRA32:
	{
		n = 4;
		break;
	}
	case SR::FrameBufferAttachmentFormat::BGR24:
	{
		n = 3;
		break;
	}
	case SR::FrameBufferAttachmentFormat::Depth32:
	{
		n = 1;
		break;
	}
	}
	return n;
}
