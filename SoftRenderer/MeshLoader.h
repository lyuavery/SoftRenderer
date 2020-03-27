#pragma once
#include "Header.h"

#include "Mesh.h"
#include "Math/Vector.h"
namespace SR
{
	class Mesh;
	class MeshLoader
	{
	public:
		static MeshLoader& Instance();
		enum class FileType {
			Obj
		};

		class Mesh* Load(const std::string& fileName);


	private:
		static MeshLoader* instance;

		MeshLoader() {}
		MeshLoader(const MeshLoader& ml) {}
		MeshLoader& operator=(const MeshLoader& ml) { return *this; }

		class Mesh* LoadObj(const std::string& fileName);

	};
}

