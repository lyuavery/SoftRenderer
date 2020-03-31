#pragma once
#include "Header.h"
#include "Texture.h"
#include "Color.h"
namespace SR
{
	enum class FrameBufferAttachmentFormat
	{
		ARGB32,
		BGRA32,
		BGR24,
		Depth8
	};

	class FrameBufferAttachment : public Texture
	{
	private:
		FrameBufferAttachmentFormat format;
	public:
		FrameBufferAttachment(FrameBufferAttachmentFormat fmt, int w, int h) :Texture(w, h, GetFormatChannels(fmt), GetFormatBytesPerPixel(fmt), nullptr), format(fmt)
		{
			buffer = new Byte[w * h * bytespp > 0 ? w * h * bytespp : 0];
		}
		FrameBufferAttachment(FrameBufferAttachmentFormat fmt, int w, int h, Byte* raw) :Texture(w, h, GetFormatChannels(fmt), GetFormatBytesPerPixel(fmt), raw), format(fmt)
		{ }
		FrameBufferAttachment(FrameBufferAttachment&& t) :Texture(t.width, t.height, GetFormatChannels(t.format), GetFormatBytesPerPixel(t.format), t.buffer), format(t.format)
		{ }
		FrameBufferAttachment() = delete;

		virtual void SetColor(int x, int y, const Color&) override;
		virtual void SetColor32(int x, int y, const Color32&) override;
		virtual Color GetColor(int x, int y) const override;
		virtual Color32 GetColor32(int x, int y) const override;
		virtual void Clear(const Color& c) override;
		virtual void Clear(const Color32& c) override;
		static int GetFormatChannels(FrameBufferAttachmentFormat fmt);
		static int GetFormatBytesPerPixel(FrameBufferAttachmentFormat fmt);
	};

	class FrameBuffer
	{
	public:
		FrameBufferAttachment colorBuf;
		FrameBufferAttachment depthBuf;

		FrameBuffer(FrameBufferAttachmentFormat colorBufFmt, int w0, int h0, FrameBufferAttachmentFormat depthBufFmt, int w1, int h1)
			:colorBuf(colorBufFmt, w0, h0), depthBuf(depthBufFmt, w1, h1)
		{}

		FrameBuffer(FrameBufferAttachmentFormat colorBufFmt, int w0, int h0, Byte* colorData, FrameBufferAttachmentFormat depthBufFmt, int w1, int h1, Byte* depthData)
			:colorBuf(colorBufFmt, w0, h0, colorData), depthBuf(depthBufFmt, w1, h1, depthData)
		{}

		void Clear(const Color& c = Color::black)
		{
			colorBuf.Clear(c);
			depthBuf.Clear(c);
		}
		void Clear(const Color32& c = Color32::black)
		{
			colorBuf.Clear(c);
			depthBuf.Clear(c);
		}
		FrameBuffer() = delete;
	};
	
}