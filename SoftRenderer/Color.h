#pragma once

#include "Math/Vector.h"
#include "TypeDef.h"
namespace SR
{
	struct Color32;
	struct Color : Vec4
	{
		Color() = default;
		explicit Color(float v) :Vec4(v) {}
		Color(float r, float g, float b) :Vec4(r, g, b, 1) {}
		Color(float r, float g, float b, float a) :Vec4(r, g, b, a) {}
		static const Color black, white, grey, zero, red, green, blue;
		//operator Color32() const;
	};

	struct Color;
	struct Color32 : Vec4uc
	{
		Color32() = default;
		explicit Color32(Byte v) :Vec4uc(v) {}
		Color32(Byte r, Byte g, Byte b) :Vec4uc(r, g, b, 255) {}
		Color32(Byte r, Byte g, Byte b, Byte a) :Vec4uc(r, g, b, a) {}
		static const Color32 black, white, grey, zero, red, green, blue;
		explicit operator SR::Color() const;
	};

}
