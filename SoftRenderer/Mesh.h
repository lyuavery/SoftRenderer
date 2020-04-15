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
		Mesh(const Mesh& m, int vertexAttribute = (int)VertexAttribute::Positions) {
			attrib = vertexAttribute;
			if ((vertexAttribute & (int)VertexAttribute::Positions) != 0)
			{
				this->vertices = m.vertices;
				this->indices = m.indices;
			}
			if ((vertexAttribute & (int)VertexAttribute::UVs) != 0)
				this->uvs = m.uvs;
			if ((vertexAttribute & (int)VertexAttribute::Tangents) != 0)
				this->tangents = m.tangents;
			if ((vertexAttribute & (int)VertexAttribute::Normals) != 0)
				this->normals = m.normals;
			if ((vertexAttribute & (int)VertexAttribute::Colors) != 0)
				this->colors = m.colors;
		}

		inline int Attribute(VertexAttribute a) {
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
			return (attrib & (int)a) && b;
		}
		inline void EnableAttribute(bool b, VertexAttribute a)
		{
			if (b)
			{
				attrib &= (int)a;
			}
			else
			{
				attrib &= ~(int)a;
			}
		}


		//std::vector<std::vector<Vec3i>> faces;
		//std::vector<UInt32> faces;
	};
}

