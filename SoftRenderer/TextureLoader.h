#pragma once
#include <string>
#include "Texture.h"
#include <map>
namespace SR
{
	enum class TextureFileType
	{
		TGA
	};

	class TextureLoader
	{
	protected:
		auto GetTextureRawBuffer(const Texture* texture) ->decltype(texture->buffer) { return texture ? texture->buffer : nullptr; }
	public:
		virtual Texture2D* Load(const char *filename) = 0;
		virtual bool Save(const char* filename, const Texture2D* const tex, bool rle = true) = 0;
	};

#pragma pack(push,1)
	struct TGA_Header
	{
		// 颜色映射相关参数；
		// 原点x、y，宽高，像素字节数，压缩方法，是否水平/垂直翻转等补充信息
		unsigned char idlength;	// Image ID length, 指定Image Identification Field的长度
		char colormaptype;		// Color map type, 一般是0或1，1表示是color-mapped imaged，0表示不是
		char datatypecode;		// Image type，image的类型，如1表示Uncompressed, color-mapped images
								/// Color Map Specification
		short colormaporigin;	// Integer index of first color map entry ( lo-hi ) 
		short colormaplength;	// Integer count of color map entries ( lo-hi ) 
		char colormapdepth;		// Number of bits in each color map entry / number of bits per pixel
								/// Image Specification
		short x_origin;			// Integer X coordinate of the lower left corner( lo-hi ) 
		short y_origin;			// Integer Y coordinate of the lower left corner( lo-hi ) 
		short width;			// Width of Image, in pixels( lo-hi ) 
		short height;			// Height of Image, in pixels( lo-hi ) 
		char  bitsperpixel;		// Image Pixel Size
		char  imagedescriptor;	// Image Descriptor Byte, 8bits,
		// Image Identification Field ,length Color Map Specification
		// Color map data
		// Image Data Field
		enum { GRAYSCALE = 1, BGR = 3, BGRA = 4 };
	};
#pragma pack(pop)

	class TGALoader :public TextureLoader
	{
		bool LoadRLEData(std::ifstream &in, const Texture2D* const tex);
		bool UnloadRLEData(std::ofstream &out, const Texture2D* const tex);
	public:
		virtual Texture2D* Load(const char *filename) override;
		virtual bool Save(const char* filename, const Texture2D* const tex, bool rle = true) override;
	};

}

