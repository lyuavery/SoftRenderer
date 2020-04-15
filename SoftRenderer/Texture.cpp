#include "Texture.h"
#include "Math/Utility.h"
void SR::Texture2D::Set(int x, int y, float r, float g, float b, float a)
{
	if (x >= width || x < 0 || y < 0 || y >= height) return;
	auto offset = (x + y * width) * bytespp;
	auto ptr = reinterpret_cast<Byte*>(buffer);
	switch (format)
	{
	case SR::TextureFormat::A8: {
		*((float*)(ptr + offset)) = r;
		break;
	}
	case SR::TextureFormat::BGR24:
	case SR::TextureFormat::RGB24:
	case SR::TextureFormat::ARGB32:
	case SR::TextureFormat::BGRA32: {
		Set(x, y, Byte(sbm::clamp01(r) * 255),
			Byte(sbm::clamp01(g) * 255),
			Byte(sbm::clamp01(b) * 255),
			Byte(sbm::clamp01(a) * 255));
		break;
	}
	}
}

void SR::Texture2D::Set(int x, int y, Byte r, Byte g, Byte b, Byte a)
{
	if (x >= width || x < 0 || y < 0 || y >= height) return;
	auto ptr = reinterpret_cast<Byte*>(buffer);
	auto offset = (x + y * width) * bytespp;
	switch (format)
	{
	case SR::TextureFormat::RGB24: {
		*(ptr + offset) = r;
		*(ptr + offset + 1) = g;
		*(ptr + offset + 2) = b;
		break;
	}
	case SR::TextureFormat::BGR24: {
		*(ptr + offset) = b;
		*(ptr + offset + 1) = g;
		*(ptr + offset + 2) = r;
		break;
	}
	case SR::TextureFormat::ARGB32: {
		*(ptr + offset + 0) = a;
		*(ptr + offset + 1) = r;
		*(ptr + offset + 2) = g;
		*(ptr + offset + 3) = b;
		break;
	}
	case SR::TextureFormat::BGRA32: {
		*(ptr + offset) = b;
		*(ptr + offset + 1) = g;
		*(ptr + offset + 2) = r;
		*(ptr + offset + 3) = a;
		break;
	}
	case SR::TextureFormat::A8: {
		Set(x, y, 0, 0, 0, a / 255.f);
		break;
	}
	}
}

int SR::Texture2D::Get(int x, int y, float& r, float& g, float& b, float& a) const
{
	if (x >= width || x < 0 || y < 0 || y >= height) return -1;
	int components = 0;
	auto ptr = reinterpret_cast<Byte*>(buffer);
	auto offset = (x + y * width) * bytespp;
	switch (format)
	{
	case SR::TextureFormat::A8:
	case SR::TextureFormat::BGR24:
	case SR::TextureFormat::RGB24:
	case SR::TextureFormat::BGRA32:
	case SR::TextureFormat::ARGB32: {
		Byte _r, _g, _b, _a;
		components = Get(x, y, _r, _g, _b, _a);
		r = _r, g = _g, b = _b, a = _a;
		break;
	}
	}
	return components;
}

int SR::Texture2D::Get(int x, int y, Byte& r, Byte& g, Byte& b, Byte& a) const
{
	if (x >= width || x < 0 || y < 0 || y >= height) return -1;
	int components = 0;
	auto ptr = reinterpret_cast<Byte*>(buffer);
	auto offset = (x + y * width) * bytespp;
	switch (format)
	{
	case SR::TextureFormat::RGB24: {
		components = 3;
		r = *(ptr + offset);
		g = *(ptr + offset + 1);
		b = *(ptr + offset + 2);
		break;
	}
	case SR::TextureFormat::BGR24: {
		components = 3;
		b = *(ptr + offset);
		g = *(ptr + offset + 1);
		r = *(ptr + offset + 2);
		break;
	}
	case SR::TextureFormat::ARGB32: {
		components = 4;
		a = *(ptr + offset);
		r = *(ptr + offset + 1);
		g = *(ptr + offset + 2);
		b = *(ptr + offset + 3);
		break;
	}
	case SR::TextureFormat::BGRA32: {
		components = 4;
		b = *(ptr + offset);
		g = *(ptr + offset + 1);
		r = *(ptr + offset + 2);
		a = *(ptr + offset + 3);
		break;
	}
	case SR::TextureFormat::A8: {
		components = 1;
		a = *(ptr + offset);
		break;
	}
	}
	return components;
}

void SR::Texture2D::Clear(float r, float g, float b, float a)
{
	if (!buffer) return;
	for (int j = 0; j < height; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			Set(i, j, r,g,b,a);
		}
	}
}

void SR::Texture2D::Clear(Byte r, Byte g, Byte b, Byte a)
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

int SR::Texture2D::GetFormatBytesPerPixel(SR::TextureFormat fmt)
{
	int n = GetFormatChannels(fmt);
	int elemSize = 0;
	switch (fmt)
	{
	case SR::TextureFormat::ARGB32:
	case SR::TextureFormat::BGRA32:
	case SR::TextureFormat::RGB24:
	case SR::TextureFormat::BGR24:
	case SR::TextureFormat::A8:
	{
		elemSize = sizeof(Byte);
	}
	}
	return n * elemSize;
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