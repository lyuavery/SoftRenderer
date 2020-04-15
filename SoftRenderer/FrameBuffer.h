#pragma once
#include "Texture.h"
#include "Color.h"
#include "TypeDef.h"
namespace SR
{
	enum class FrameBufferAttachmentFormat
	{
		ARGB32,
		BGRA32,
		BGR24,
		Depth32
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

		/*virtual void SetColor(int x, int y, const Color&) override;
		virtual void SetColor32(int x, int y, const Color32&) override;
		virtual Color GetColor(int x, int y) const override;
		virtual Color32 GetColor32(int x, int y) const override;
		virtual void Clear(const Color& c) override;
		virtual void Clear(const Color32& c) override;*/
		virtual void Set(int x, int y, Byte, Byte, Byte, Byte);
		virtual void Set(int x, int y, float, float, float, float);
		virtual int Get(int x, int y, Byte&, Byte&, Byte&, Byte&) const;
		virtual int Get(int x, int y, float&, float&, float&, float&) const;
		virtual void Clear(Byte, Byte, Byte, Byte);
		virtual void Clear(float, float, float, float);

		static int GetFormatChannels(FrameBufferAttachmentFormat fmt);
		static int GetFormatBytesPerPixel(FrameBufferAttachmentFormat fmt);

	};

	class FrameBuffer
	{
	public:
		std::unique_ptr<Texture> colorBuf;
		std::unique_ptr<Texture> depthBuf;
		std::unique_ptr<Texture> colorBuf1;

		//FrameBuffer(FrameBufferAttachmentFormat colorBufFmt, int w0, int h0, FrameBufferAttachmentFormat depthBufFmt, int w1, int h1)
		//	:colorBuf(colorBufFmt, w0, h0), depthBuf(depthBufFmt, w1, h1)
		//{}

		//FrameBuffer(FrameBufferAttachmentFormat colorBufFmt, int w0, int h0, Byte* colorData, FrameBufferAttachmentFormat depthBufFmt, int w1, int h1, Byte* depthData)
		//	:colorBuf(colorBufFmt, w0, h0, colorData), depthBuf(depthBufFmt, w1, h1, depthData)
		//{}

		//void Clear(const Color& c = Color::black, const Color& d = Color::black)
		//{
		//	if (colorBuf) colorBuf->Clear(c);
		//	if (depthBuf) depthBuf->Clear(d);
		//}
		//void Clear(const Color32& c = Color32::black, const Color32& d = Color32::black)
		//{
		//	if (colorBuf) colorBuf->Clear(c);
		//	if (depthBuf) depthBuf->Clear(d);
		//}
		FrameBuffer() = delete;
	};
	
}