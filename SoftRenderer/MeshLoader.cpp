#include <iostream>

#include <string>
#include <iterator>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <fstream>

#include "Header.h"
#include "MeshLoader.h"
#include "Mesh.h"

using namespace SR;
MeshLoader* MeshLoader::instance = nullptr;

MeshLoader& MeshLoader::Instance() {
	return *instance;
}

Mesh* MeshLoader::Load(const std::string& fileName)
{
	if (fileName.empty())
		return nullptr;
	auto pos = fileName.rfind('.');
	if (pos == std::string::npos) {
		XLogWarning("Unknown file ext: %s", fileName.c_str());
		return nullptr;
	}
	std::string ext = fileName.substr(pos + 1, fileName.length() - pos - 1);
	for (auto& c : ext) {
		c = tolower(c);
	}
	if (ext == "obj") {
		return LoadObj(fileName);
	}
	else {
		XLogWarning("Unsopported file ext: %s", ext.c_str());
		return nullptr;
	}
}

Mesh* MeshLoader::LoadObj(const std::string& fileName)
{
	std::ifstream in;
	in.open(fileName, std::ifstream::in);
	if (!in.is_open()) {
		XLogError("Fail to open file: %s.", fileName.c_str());
		return nullptr;
	}
	Mesh* mesh = new Mesh;
	std::string line;
	while (!in.eof()) {
		std::getline(in, line);
		std::istringstream iss(line);
		char trash;
		if (!line.compare(0, 2, "v ")) {
			iss >> trash;
			Vec3 v;
			for (int i = 0; i < 3; i++) iss >> v[i];
			mesh->vertices.push_back(v);
		}
		else if (!line.compare(0, 3, "vn ")) {
			iss >> trash >> trash;
			Vec3 n;
			for (int i = 0; i < 3; i++) iss >> n[i];
			mesh->normals.push_back(n);
		}
		else if (!line.compare(0, 3, "vt ")) {
			iss >> trash >> trash;
			Vec2 uv;
			for (int i = 0; i < 2; i++) iss >> uv[i];
			mesh->uv.push_back(uv);
		}
		else if (!line.compare(0, 2, "f ")) {
			std::vector<Vec3i> f;
			Vec3i tmp;
			iss >> trash;
			while (iss >> tmp[0] >> trash >> tmp[1] >> trash >> tmp[2]) {
				for (int i = 0; i < 3; i++) tmp[i]--; // in wavefront obj all indices start at 1, not zero
				f.push_back(tmp);
			}
			mesh->faces.push_back(f);
		}
	}
	XLogInfo("Load mesh successfully: %s, v# %d, f# %d, vt# %d, vn# %d\n", fileName.c_str(), mesh->vertices.size(), mesh->faces.size(), mesh->uv.size(), mesh->normals.size());
	return mesh;
}
