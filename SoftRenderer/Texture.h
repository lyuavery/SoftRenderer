#pragma once
#include <memory>

#include "Header.h"
#include "Color.h"

namespace SR
{

	enum class TextureFormat
	{
		RGB24,
		BGR24,
		ARGB32,
		BGRA32,
		A8,
	};

	class Texture
	{
	protected:
		int width;
		int height;
		int channels;
		int bytespp;
		Byte* buffer; 
		Texture(int w, int h, int ch, int bpp, Byte* buf) :width(w),height(h),channels(ch),bytespp(bpp),buffer(buf) {}
	public:
		virtual void Assign(Byte* data)  { buffer = data; }
		virtual int GetWidth() const  { return width; }
		virtual int GetHeight() const  { return height; }
		virtual int GetBytesPerPixel() const  { return bytespp; }
		virtual int GetChannels() const  { return channels; }

		virtual void SetColor(int x, int y, const Color&) = 0;
		virtual void SetColor32(int x, int y, const Color32&) = 0;
		virtual inline Color GetColor(int x, int y) const = 0;
		virtual inline Color32 GetColor32(int x, int y) const = 0;
		virtual void Clear(const Color& c = Color::black) = 0;
		virtual void Clear(const Color32& c = Color32::black) = 0;
		
		virtual ~Texture() {
			if (buffer) {
				delete[] buffer;
			}
		}
		friend class TextureLoader;
	};

	class Texture2D : public Texture
	{
		TextureFormat format;
	public:
		Texture2D(TextureFormat fmt, int w, int h) :Texture(w,h, GetFormatChannels(fmt), GetFormatBytesPerPixel(fmt), nullptr), format(fmt)
		{
			buffer = new Byte[w * h * bytespp > 0 ? w * h * bytespp : 0];
		}
		Texture2D(TextureFormat fmt, int w, int h, Byte* raw) :Texture(w, h, GetFormatChannels(fmt), GetFormatBytesPerPixel(fmt), raw), format(fmt)
		{ }
		Texture2D(Texture2D&& t) :Texture(t.width, t.height, GetFormatChannels(t.format), GetFormatBytesPerPixel(t.format), t.buffer), format(t.format)
		{ }
		
		virtual void SetColor(int x, int y, const Color&) override;
		virtual void SetColor32(int x, int y, const Color32&) override;
		virtual Color GetColor(int x, int y) const override;
		virtual Color32 GetColor32(int x, int y) const override;
		virtual void Clear(const Color& c) override;
		virtual void Clear(const Color32& c) override;
		void FlipVertically();
		void FlipHorizontally();

		static int GetFormatChannels(TextureFormat fmt);
		static int GetFormatBytesPerPixel(TextureFormat fmt);
		TextureFormat GetFormat() const { return format; }
	};

}

