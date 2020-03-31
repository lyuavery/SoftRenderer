#include <fstream>
#include "TextureLoader.h"
#include "Texture.h"
#include "Log.h"

SR::Texture2D* SR::TGALoader::Load(const char *filename)
{
	if (!filename) return nullptr;
	//if (data) delete[] data;
	//data = NULL;
	std::ifstream in;
	in.open(filename, std::ios::binary);
	if (!in.is_open()) {
		in.close();
		XLogWarning("Can't open file: \"%s\".\n", filename);
		return nullptr;
	}
	SR::TGA_Header header;
	in.read((char *)&header, sizeof(header));
	if (!in.good()) {
		in.close();
		XLogWarning("An error occured while reading \"%s\"'s header.\n", filename);
		return nullptr;
	}

	int width = header.width, 
		height = header.height,
		bytespp = header.bitsperpixel >> 3; // bytes per texel

	if (width <= 0 || height <= 0 || (bytespp != SR::TGA_Header::GRAYSCALE && bytespp != SR::TGA_Header::BGR && bytespp != SR::TGA_Header::BGRA)) {
		in.close();
		XLogWarning("Bad header values for \"%s\"'.\n", filename);
		return nullptr;
	}
	SR::TextureFormat fmt;
	switch (bytespp)
	{
	case SR::TGA_Header::GRAYSCALE: { fmt = SR::TextureFormat::A8; break; }
	case SR::TGA_Header::BGR: { fmt = SR::TextureFormat::BGR24; break; }
	case SR::TGA_Header::BGRA: { fmt = SR::TextureFormat::BGRA32; break; }
	}
	SR::Texture2D* tex = new SR::Texture2D(fmt, width, height);
	assert(bytespp == tex->GetBytesPerPixel());
	unsigned long nbytes = bytespp * width * height;

	auto data = GetTextureRawBuffer(tex);
	if (3 == header.datatypecode || 2 == header.datatypecode) {
		in.read((char *)data, nbytes);
		if (!in.good()) {
			in.close();
			XLogWarning("An error occured while reading \"%s\"'s data.\n", filename);
			return false;
		}
	}
	else if (10 == header.datatypecode || 11 == header.datatypecode) {
		if (!LoadRLEData(in, tex)) {
			in.close();
			XLogWarning("An error occured while reading \"%s\"'s data.\n", filename);
			return false;
		}
	}
	else {
		in.close();
		XLogWarning("Unknown file format:%d.\n", (int)header.datatypecode);
		return false;
	}

	
	if (!(header.imagedescriptor & 0x20)) {
		tex->FlipVertically();
	}
	if (header.imagedescriptor & 0x10) {
		tex->FlipHorizontally();
	}
	in.close();
	XLogInfo("Texture loaded:[%s:%dx%d, bpp:%d].\n", filename, width, height, bytespp);

	return tex;
}

bool SR::TGALoader::Save(const char* filename, const SR::Texture2D* const texture, bool rle)
{
	if (!texture || !filename) return false;
	auto& tex = *texture;
	unsigned char developer_area_ref[4] = { 0, 0, 0, 0 };
	unsigned char extension_area_ref[4] = { 0, 0, 0, 0 };
	unsigned char footer[18] = { 'T','R','U','E','V','I','S','I','O','N','-','X','F','I','L','E','.','\0' };
	std::ofstream out;
	out.open(filename, std::ios::binary);
	if (!out.is_open()) {
		out.close();
		XLogWarning("Can't open file: \"%s\".\n", filename);
		return false;
	}
	SR::TGA_Header header;
	int bytespp = tex.GetBytesPerPixel();
	int width = tex.GetWidth();
	int height = tex.GetHeight();
	auto data = GetTextureRawBuffer(texture);
	memset((void *)&header, 0, sizeof(header));
	header.bitsperpixel = bytespp << 3;
	header.width = width;
	header.height = height;
	header.datatypecode = (bytespp == SR::TGA_Header::GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2));
	header.imagedescriptor = 0x20; // top-left origin
	out.write((char *)&header, sizeof(header));
	if (!out.good()) {
		out.close();
		XLogWarning("Can't dump the tga file: \"%s\".\n", filename);
		return false;
	}
	if (!rle) {
		out.write((char *)data, width*height*bytespp);
		if (!out.good()) {
			out.close();
			XLogWarning("Can't unload raw data: \"%s\".\n", filename);
			return false;
		}
	}
	else {
		if (!UnloadRLEData(out, texture)) {
			out.close();
			XLogWarning("Can't unload rle data: \"%s\".\n", filename);
			return false;
		}
	}
	out.write((char *)developer_area_ref, sizeof(developer_area_ref));
	if (!out.good()) {
		out.close();
		XLogWarning("Can't dump the tga file: \"%s\".\n", filename);
		return false;
	}
	out.write((char *)extension_area_ref, sizeof(extension_area_ref));
	if (!out.good()) {
		out.close();
		XLogWarning("Can't dump the tga file: \"%s\".\n", filename);
		return false;
	}
	out.write((char *)footer, sizeof(footer));
	if (!out.good()) {
		out.close();
		XLogWarning("Can't dump the tga file: \"%s\".\n", filename);
		return false;
	}
	out.close();
	return true;
}

bool SR::TGALoader::LoadRLEData(std::ifstream &in, const Texture2D* const texture)
{
	if (!texture) return false;
	const Texture& tex = *texture;
	auto data = GetTextureRawBuffer(texture);
	auto bytespp = tex.GetBytesPerPixel();
	unsigned long pixelcount = tex.GetWidth() *  tex.GetHeight();
	unsigned long currentpixel = 0;
	unsigned long currentbyte = 0;
	auto colorbuffer = new std::remove_pointer<decltype(data)>::type[bytespp];
	do {
		unsigned char chunkheader = 0;
		chunkheader = in.get();
		if (!in.good()) {
			XLogWarning("An error occured while reading the data.\n");
			return false;
		}
		if (chunkheader < 128) {
			chunkheader++;
			for (int i = 0; i < chunkheader; i++) {
				in.read((char *)colorbuffer, bytespp);
				if (!in.good()) {
					XLogWarning("An error occured while reading the header.\n");
					return false;
				}
				for (int t = 0; t < bytespp; t++)
					data[currentbyte++] = colorbuffer[t];
				currentpixel++;
				if (currentpixel > pixelcount) {
					XLogWarning("Too many pixels read.\n");
					return false;
				}
			}
		}
		else {
			chunkheader -= 127;
			in.read((char *)colorbuffer, bytespp);
			if (!in.good()) {
				XLogWarning("An error occured while reading the header.\n");
				return false;
			}
			for (int i = 0; i < chunkheader; i++) {
				for (int t = 0; t < bytespp; t++)
					data[currentbyte++] = colorbuffer[t];
				currentpixel++;
				if (currentpixel > pixelcount) {
					XLogWarning("Too many pixels read.\n");
					return false;
				}
			}
		}
	} while (currentpixel < pixelcount);
	return true;
}

// 语意：希望Texture的raw buf只对实现了某个接口的类开放
bool SR::TGALoader::UnloadRLEData(std::ofstream &out, const Texture2D* const texture)
{
	if (!texture) return false;
	const Texture& tex = *texture;
	auto data = GetTextureRawBuffer(texture);
	const unsigned char max_chunk_length = 128;
	unsigned long npixels = tex.GetWidth() * tex.GetHeight();
	auto bytespp = tex.GetBytesPerPixel();
	unsigned long curpix = 0;
	while (curpix < npixels) {
		unsigned long chunkstart = curpix * bytespp;
		unsigned long curbyte = curpix * bytespp;
		unsigned char run_length = 1;
		bool raw = true;
		while (curpix + run_length < npixels && run_length < max_chunk_length) {
			bool succ_eq = true;
			for (int t = 0; succ_eq && t < bytespp; t++) {
				succ_eq = (data[curbyte + t] == data[curbyte + t + bytespp]);
			}
			curbyte += bytespp;
			if (1 == run_length) {
				raw = !succ_eq;
			}
			if (raw && succ_eq) {
				run_length--;
				break;
			}
			if (!raw && !succ_eq) {
				break;
			}
			run_length++;
		}
		curpix += run_length;
		//out << (raw ? run_length - 1 : run_length + 127);
		out.put(raw ? run_length - 1 : run_length + 127);
		if (!out.good()) {
			XLogWarning("Can't dump the tga file.\n");
			return false;
		}
		out.write((char *)(data + chunkstart), (raw ? run_length * bytespp : bytespp));
		if (!out.good()) {
			XLogWarning("Can't dump the tga file.\n");
			return false;
		}
	}
	return true;
}