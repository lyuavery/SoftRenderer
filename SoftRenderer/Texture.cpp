#include "Texture.h"
#include"Math/Utility.h"
void SR::Texture2D::SetColor(int x, int y, const SR::Color& color)
{
	SR::Color32 color32 = SR::Color32(
		sbm::clamp01(color.r) * 255,
		sbm::clamp01(color.g) * 255,
		sbm::clamp01(color.b) * 255,
		sbm::clamp01(color.a) * 255
	);
	SetColor32(x, y, color32);
}

void SR::Texture2D::SetColor32(int x, int y, const SR::Color32& color32)
{
	auto ptr = reinterpret_cast<Byte*>(buffer);
	auto offset = (x + y * width) * bytespp;
	switch (format)
	{
	case SR::TextureFormat::RGB24: {
		*(ptr + offset) = color32.r;
		*(ptr + offset + 1) = color32.g;
		*(ptr + offset + 2) = color32.b;
		break;
	}
	case SR::TextureFormat::BGR24: {
		*(ptr + offset) = color32.b;
		*(ptr + offset + 1) = color32.g;
		*(ptr + offset + 2) = color32.r;
		break;
	}
	case SR::TextureFormat::ARGB32: {
		memcpy((void*)(ptr + offset), &color32, bytespp);
		break;
	}
	case SR::TextureFormat::BGRA32: {
		*(ptr + offset) = color32.b;
		*(ptr + offset + 1) = color32.g;
		*(ptr + offset + 2) = color32.r;
		*(ptr + offset + 3) = color32.a;
		break;
	}
	case SR::TextureFormat::A8: {
		*(ptr + offset) = color32.r;
		break;
	}
	default:
		break;
	}
}

SR::Color SR::Texture2D::GetColor(int x, int y) const
{
	Color color = Color(GetColor32(x, y));
	return color;
}

SR::Color32 SR::Texture2D::GetColor32(int x, int y) const
{
	Color32 color32(255);
	auto ptr = reinterpret_cast<Byte*>(buffer);
	auto offset = (x + y * width) * bytespp;
	switch (format)
	{
	case SR::TextureFormat::RGB24: {
		color32.r = *(ptr + offset);
		color32.g = *(ptr + offset + 1);
		color32.b = *(ptr + offset + 2);
	}
								   break;
	case SR::TextureFormat::BGR24: {
		color32.b = *(ptr + offset);
		color32.g = *(ptr + offset + 1);
		color32.r = *(ptr + offset + 2);
	}
								   break;
	case SR::TextureFormat::ARGB32: {
		color32.a = *(ptr + offset);
		color32.r = *(ptr + offset + 1);
		color32.g = *(ptr + offset + 2);
		color32.b = *(ptr + offset + 3);
	}
									break;
	case SR::TextureFormat::BGRA32: {
		color32.b = *(ptr + offset);
		color32.g = *(ptr + offset + 1);
		color32.r = *(ptr + offset + 2);
		color32.a = *(ptr + offset + 3);
	}
		break;
	case SR::TextureFormat::A8: {
		color32 = SR::Color32(*((ptr + offset)));
	}
		break;
	default:
		break;
	}
	return color32;
}

void SR::Texture2D::Clear(const Color& color)
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

void SR::Texture2D::Clear(const Color32& color32)
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

int SR::Texture2D::GetFormatBytesPerPixel(SR::TextureFormat fmt)
{
	int m = 0;
	int n = GetFormatChannels(fmt);
	switch (fmt)
	{
	case SR::TextureFormat::ARGB32:
	case SR::TextureFormat::BGRA32:
	case SR::TextureFormat::RGB24:
	case SR::TextureFormat::BGR24:
	case SR::TextureFormat::A8:
	{
		m = n * sizeof(Byte);
	}
	/*case Format::ARGBHalf:
	case Format::ZHalf:
	{
		m = n * sizeof(float);
		break;
	}*/
	}
	return m;
}

int SR::Texture2D::GetFormatChannels(SR::TextureFormat fmt)
{
	int n = 0;
	switch (fmt)
	{
	case SR::TextureFormat::ARGB32:
	case SR::TextureFormat::BGRA32:
	{
		n = 4;
		break;
	}
	case SR::TextureFormat::RGB24:
	case SR::TextureFormat::BGR24:
	{
		n = 3;
		break;
	}
	case SR::TextureFormat::A8:
	{
		n = 1;
		break;
	}
	/*case Format::ARGBHalf:
	{
		n = 4;
		break;
	}
	case Format::ZHalf:
	{
		n = 1;
		break;
	}*/
	}
	return n;
}

void SR::Texture2D::FlipVertically()
{
	if (!buffer) return;
	const UInt32 bytePerLine = width * channels;
	auto line = new Byte[bytePerLine];
	int half = height >> 1;
	for (int j = 0; j < half; j++) {
		UInt32 l1 = j * bytePerLine;
		UInt32 l2 = (height - 1 - j)* bytePerLine;
		memmove((void *)line, (void *)(buffer + l1), bytePerLine);
		memmove((void *)(buffer + l1), (void *)(buffer + l2), bytePerLine);
		memmove((void *)(buffer + l2), (void *)line, bytePerLine);
	}
	delete[] line;
}

void SR::Texture2D::FlipHorizontally()
{
	if (!buffer) return;
	int half = width >> 1;
	const int bytePerLine = width * channels;
	auto tmp = new Byte[channels];
	for (int i = 0; i < half; i++) {
		for (int j = 0; j < height; j++) {
			auto p1 = buffer + i + j * bytePerLine;
			auto p2 = buffer + (width - i) + j * bytePerLine;
			memmove((void *)tmp, (void *)p1, channels);
			memmove((void *)p1, (void *)p2, channels);
			memmove((void *)p2, (void *)tmp, channels);
		}
	}
	delete[] tmp;
}