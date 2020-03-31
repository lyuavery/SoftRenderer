#include "Color.h"

const SR::Color SR::Color::black = Color(0.f, 0.f, 0.f, 1.0f), 
SR::Color::white = Color(1.0f),
SR::Color::grey = Color(.5f,.5f,.5f,1),
SR::Color::zero = Color(.0f);

const SR::Color32 SR::Color32::black = Color32(0, 0, 0, 255), 
SR::Color32::white = Color32(255), 
SR::Color32::grey = Color32(128,128,128,255),
SR::Color32::zero = Color32(0);

SR::Color32::operator SR::Color() const
{
	const float denominator = 1.f / 255;
	return Color(r * denominator, g * denominator, b * denominator, a * denominator);
}
//
//SR::Color::operator SR::Color32() const
//{
//	UInt32 _r = int(0.5f + 255 * r); _r = _r > 255 ? 255 : _r;
//	UInt32 _g = int(0.5f + 255 * g); _g = _g > 255 ? 255 : _g;
//	UInt32 _b = int(0.5f + 255 * b); _b = _b > 255 ? 255 : _b;
//	UInt32 _a = int(0.5f + 255 * a); _a = _a > 255 ? 255 : _a;
//	return Color32(_r, _g, _b, _a);
//}
