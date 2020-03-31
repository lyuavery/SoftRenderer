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
		static MeshLoader& GetInstance()
		{
			static MeshLoader* instance = new MeshLoader;
			return *instance;
		}

		enum class FileType {
			OBJ
		};

		class Mesh* Load(const std::string& fileName);


	private:

		MeshLoader() {}
		MeshLoader(const MeshLoader& ml) {}
		MeshLoader& operator=(const MeshLoader& ml) { return *this; }

		class Mesh* LoadObj(const std::string& fileName);

	};
}

