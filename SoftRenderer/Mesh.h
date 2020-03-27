#pragma once
#include "Math/Vector.h"
#include <vector>
#include <istream>

namespace SR
{
	class Mesh {
	public:
		std::vector<Vec3> vertices;
		std::vector<Vec2> uv;
		std::vector<Vec3> normals;
		std::vector<Vec3> indices;
		std::vector<std::vector<Vec3i>> faces;
	};
}

