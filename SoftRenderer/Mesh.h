#pragma once
#include "Math/Vector.h"
#include "TypeDef.h"
#include "Color.h"
#include "VertexShader.h"
#include <vector>
#include <istream>

namespace SR
{
	enum class VertexAttribute
	{
		None = 0,
		Positions = 1 << 1,
		Normals = 1 << 2,
		UVs = 1 << 3,
		Colors = 1 << 4,
		Tangents = 1 << 5,
	};

	class Mesh {
		int attrib = (int)VertexAttribute::Positions;
	public:
		std::vector<Vec3> vertices;
		std::vector<Vec2> uvs;
		std::vector<Vec3> normals;
		std::vector<Vec4> tangents;
		std::vector<Color> colors;
		std::vector<UInt32> indices;
		Mesh() = default;
		Mesh(const Mesh& m, int vertexAttribute) {
			attrib = vertexAttribute == -1 ? m.attrib : vertexAttribute;
			if ((attrib & (int)VertexAttribute::Positions) != 0)
			{
				this->vertices = m.vertices;
				this->indices = m.indices;
			}
			if ((attrib & (int)VertexAttribute::UVs) != 0)
				this->uvs = m.uvs;
			if ((attrib & (int)VertexAttribute::Tangents) != 0)
				this->tangents = m.tangents;
			if ((attrib & (int)VertexAttribute::Normals) != 0)
				this->normals = m.normals;
			if ((attrib & (int)VertexAttribute::Colors) != 0)
				this->colors = m.colors;
		}

		inline bool IsValidAttribute(VertexAttribute a) {
			bool b = false;
			switch (a)
			{
			case VertexAttribute::Positions:
			{
				b = vertices.empty();
				break;
			}
			case VertexAttribute::UVs:
			{
				b = uvs.empty();
				break;
			}
			case VertexAttribute::Normals:
			{
				b = normals.empty();
				break;
			}
			case VertexAttribute::Tangents:
			{
				b = tangents.empty();
				break;
			}
			case VertexAttribute::Colors:
			{
				b = colors.empty();
				break;
			}
			}
			return (attrib & (int)a) && !b;
		}

		inline void EnableAttribute(bool b, VertexAttribute a)
		{
			if (b)
			{
				attrib |= int(a);
			}
			else
			{
				attrib &= ~int(a);
			}
		}


		//std::vector<std::vector<Vec3i>> faces;
		//std::vector<UInt32> faces;
	};
}

