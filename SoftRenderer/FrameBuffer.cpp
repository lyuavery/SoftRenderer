#include "FrameBuffer.h"

#include "Math/Utility.h"

void SR::FrameBufferAttachment::SetColor(int x, int y, const SR::Color& color)
{
	SR::Color32 color32 = SR::Color32(
		sbm::clamp01(color.r) * 255,
		sbm::clamp01(color.g) * 255,
		sbm::clamp01(color.b) * 255,
		sbm::clamp01(color.a) * 255
	);
	SetColor32(x,y, color32);
}

void SR::FrameBufferAttachment::SetColor32(int x, int y, const SR::Color32& color32)
{
	/*SR::Color32 color32 = SR::Color32(
		sbm::clamp01(color.r) * 255,
		sbm::clamp01(color.g) * 255,
		sbm::clamp01(color.b) * 255,
		sbm::clamp01(color.a) * 255
	);*/
	auto offset = (x + y * width) * bytespp;
	auto ptr = reinterpret_cast<Byte*>(buffer);
	switch (format)
	{
	case SR::FrameBufferAttachmentFormat::BGR24: {
		*(ptr + offset) = color32.b;
		*(ptr + offset + 1) = color32.g;
		*(ptr + offset + 2) = color32.r;
		break;
	}
	case SR::FrameBufferAttachmentFormat::BGRA32: {
		*(ptr + offset) = color32.b;
		*(ptr + offset + 1) = color32.g;
		*(ptr + offset + 2) = color32.r;
		*(ptr + offset + 3) = color32.a;
		break;
	}
	case SR::FrameBufferAttachmentFormat::ARGB32: {
		*(ptr + offset) = color32.a;
		*(ptr + offset + 1) = color32.r;
		*(ptr + offset + 2) = color32.g;
		*(ptr + offset + 3) = color32.b;
		break;
	}
	case SR::FrameBufferAttachmentFormat::Depth8: {
		*(ptr + offset) = color32.r;
		break;
	}
	default:
		break;
	}
}

SR::Color SR::FrameBufferAttachment::GetColor(int x, int y) const
{
	Color color = Color(GetColor32(x,y));
	return color;
}

SR::Color32 SR::FrameBufferAttachment::GetColor32(int x, int y) const
{
	Color32 color32(255);
	auto ptr = reinterpret_cast<Byte*>(buffer);
	auto offset = (x + y * width) * bytespp;
	switch (format)
	{
	case SR::FrameBufferAttachmentFormat::BGR24: {
		color32.b = *(ptr + offset);
		color32.g = *(ptr + offset + 1);
		color32.r = *(ptr + offset + 2);
		break;
	}
	case SR::FrameBufferAttachmentFormat::BGRA32: {
		color32.b = *(ptr + offset);
		color32.g = *(ptr + offset + 1);
		color32.r = *(ptr + offset + 2);
		color32.a = *(ptr + offset + 3);
		break;
	}
	case SR::FrameBufferAttachmentFormat::ARGB32: {
		color32.a = *(ptr + offset);
		color32.r = *(ptr + offset + 1);
		color32.g = *(ptr + offset + 2);
		color32.b = *(ptr + offset + 3);
		break;
	}
	case SR::FrameBufferAttachmentFormat::Depth8: {
		color32 = SR::Color32(*(ptr + offset));
		break;
	}
	default:
		break;
	}
	return color32;
}

void SR::FrameBufferAttachment::Clear(const Color& color)
{
	if (!buffer) return;
	for (int j = 0; j < height; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			SetColor(i, j, color);
		}
	}
}

void SR::FrameBufferAttachment::Clear(const Color32& color32)
{
	if (!buffer) return;
	for (int j = 0; j < height; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			SetColor32(i, j, color32);
		}
	}
}

int SR::FrameBufferAttachment::GetFormatBytesPerPixel(SR::FrameBufferAttachmentFormat fmt)
{
	int m = 0;
	int n = GetFormatChannels(fmt);
	switch (fmt)
	{
	case SR::FrameBufferAttachmentFormat::BGRA32:
	case SR::FrameBufferAttachmentFormat::ARGB32:
	case SR::FrameBufferAttachmentFormat::BGR24:
	case SR::FrameBufferAttachmentFormat::Depth8:
	{
		m = n * sizeof(Byte);
	}
	}
	return m;
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
	case SR::FrameBufferAttachmentFormat::Depth8:
	{
		n = 1;
		break;
	}
	}
	return n;
}
