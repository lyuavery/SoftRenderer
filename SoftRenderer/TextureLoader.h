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
		// ��ɫӳ����ز�����
		// ԭ��x��y����ߣ������ֽ�����ѹ���������Ƿ�ˮƽ/��ֱ��ת�Ȳ�����Ϣ
		unsigned char idlength;	// Image ID length, ָ��Image Identification Field�ĳ���
		char colormaptype;		// Color map type, һ����0��1��1��ʾ��color-mapped imaged��0��ʾ����
		char datatypecode;		// Image type��image�����ͣ���1��ʾUncompressed, color-mapped images
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

