#include <iostream>

#include <string>
#include <iterator>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include "Header.h"
#include "Log.h"
#include "MeshLoader.h"
#include "Mesh.h"
#include "Math/Matrix.h"
#include "Math/Matrix4x4.h"
#include "Math/Vector.h"

SR::Mesh* SR::MeshLoader::Load(const std::string& fileName, bool bGenerateTangents, bool bGenerateNormals)
{
	Mesh* mesh = nullptr;
	do
	{
		if (fileName.empty()) break;
		auto pos = fileName.rfind('.');
		if (pos == std::string::npos) {
			XLogWarning("Unknown file ext: %s", fileName.c_str());
		}
		std::string ext = fileName.substr(pos + 1, fileName.length() - pos - 1);
		for (auto& c : ext) {
			c = tolower(c);
		}
		if (ext == "obj") {
			return LoadObj(fileName, bGenerateNormals);
		}
		else {
			XLogWarning("Unsopported file ext: %s", ext.c_str());
			return nullptr;
		}
	} while (false);
	if (bGenerateTangents)
	{
		mesh->EnableAttribute(true, VertexAttribute::Tangents);
		GenerateTangents(mesh);
	}
	if (mesh)
	{
		mesh->indices.shrink_to_fit();
		mesh->vertices.shrink_to_fit();
		mesh->uvs.shrink_to_fit();
		mesh->normals.shrink_to_fit();
		mesh->tangents.shrink_to_fit();
	}
	return mesh;
}

SR::Mesh* SR::MeshLoader::LoadObj(const std::string& fileName, bool bGenerateNormals)
{
	std::ifstream in;
	in.open(fileName, std::ifstream::in);
	if (!in.is_open()) {
		XLogError("Fail to open file: %s.", fileName.c_str());
		return nullptr;
	}

	SR::Mesh* mesh = new SR::Mesh;
	std::string line;
	std::vector<UInt32> uvIndices;
	std::vector<UInt32> normalIndices;
	bool hasVertices = false;
	bool hasIndices = false;
	bool bRemap = false;
	while (!in.eof()) {
		std::getline(in, line);
		std::istringstream iss(line);
		char trash;
		if (!line.compare(0, 2, "v ")) {
			hasVertices = true;
			iss >> trash;
			Vec3 v;
			for (int i = 0; i < 3; i++) iss >> v[i];
			mesh->vertices.push_back(v);
		}
		else if (!line.compare(0, 3, "vn ")) {
			if (bGenerateNormals) continue;
			mesh->EnableAttribute(true, VertexAttribute::Normals);
			iss >> trash >> trash;
			Vec3 n;
			for (int i = 0; i < 3; i++) iss >> n[i];
			mesh->normals.push_back(n);
		}
		else if (!line.compare(0, 3, "vt ")) {
			mesh->EnableAttribute(true, VertexAttribute::UVs);
			iss >> trash >> trash;
			Vec2 uv;
			for (int i = 0; i < 2; i++) iss >> uv[i];
			mesh->uvs.push_back(uv);
		}
		else if (!line.compare(0, 2, "f ")) {
			hasIndices = true;
			int v0,v1,v2;
			iss >> trash;
			while (iss >> v0 >> trash >> v1 >> trash >> v2) {
				--v0; --v1; --v2;
				mesh->indices.push_back(v0);
				if (v0 != v1 || v0 != v2 || v1 != v2) bRemap = true;
				uvIndices.push_back(v1);
				if (!bGenerateNormals)
				{
					normalIndices.push_back(v2);
				}
			}
		}
	}
	if (!(hasVertices && hasIndices))
	{
		XLogWarning("Invalid obj fmt: %s.\n", fileName.c_str());
		return nullptr;
	}

	// obj文件不同顶点属性可能会有不同的索引值，而IBO只能放一个类型的索引。以最大的一个属性的索引为准，重新调整其他属性的数据顺序
	if (bRemap)
	{
		int maxsize = 0;
		int maxi = -1;
		int ss[] = {
			mesh->vertices.size(), 
			mesh->uvs.size(),
			!bGenerateNormals ? mesh->normals.size() : 0
		};
		std::vector<UInt32>* ptrs[] = { &mesh->indices, &uvIndices, &normalIndices };
		for (int i = 0, n = sizeof(ss) / sizeof(int); i < n; ++i)
		{
			if (ss[i] > maxsize)
			{
				maxsize = ss[i];
				maxi = i;
			}
		}
		if (maxi < 0)
		{
			XLogError("Unknown Error while loading mesh: %s.\n", fileName.c_str());
			return nullptr;
		}

		int mask = 7 & ~(1 << maxi);
		bool notPosMain = bool(mask & 1);
		bool notUVMain = bool(mask & 2);
		bool notNMain = bool(mask & 4);
		std::vector<UInt32>& ref = *ptrs[maxi];
		std::vector<Vec3> verRemap;
		std::vector<Vec2> uvRemap;
		std::vector<Vec3> nRemap;
		if (notPosMain) verRemap.resize(maxsize, Vec3(0));
		if (notUVMain) uvRemap.resize(maxsize, Vec2(0));
		if (notNMain) nRemap.resize(maxsize, Vec3(0));
		for (int i = 0, n = ref.size(); i < n; ++i)
		{
			if (notPosMain) verRemap[ref[i]] = mesh->vertices[mesh->indices[i]];
			if (notUVMain) uvRemap[ref[i]] = mesh->uvs[uvIndices[i]];
			if (notNMain) nRemap[ref[i]] = mesh->normals[normalIndices[i]];
		}
		if (notPosMain) mesh->vertices.swap(verRemap);
		if (notUVMain) mesh->uvs.swap(uvRemap);
		if (notNMain) mesh->normals.swap(nRemap);
		mesh->indices.swap(ref);
	}
	
	XLogInfo("Load mesh successfully: %s, v# %d, f# %d, vt# %d, vn# %d\n", fileName.c_str(), mesh->vertices.size(), mesh->indices.size(), mesh->uvs.size(), mesh->normals.size());
	return mesh;
}

void SR::MeshLoader::GenerateTangents(SR::Mesh* mesh)
{
	if (!mesh) return;
	const auto& f = mesh->indices;
	for (int i = 0, n = mesh->indices.size(); i < n; i += 3)
	{
		const auto& pos0 = mesh->vertices[f[i + 0]];
		const auto& pos1 = mesh->vertices[f[i + 1]];
		const auto& pos2 = mesh->vertices[f[i + 2]];

		const auto& uv0 = mesh->uvs[f[i + 1]];
		const auto& uv1 = mesh->uvs[f[i + 1]];
		const auto& uv2 = mesh->uvs[f[i + 1]];

		const auto& n0 = mesh->normals[f[i + 2]];
		const auto& n1 = mesh->normals[f[i + 2]];
		const auto& n2 = mesh->normals[f[i + 2]];

		sbm::Matrix<2, 2> uvMat({ uv1.x - uv0.x, uv2.x - uv0.x,uv1.y - uv0.y, uv2.y - uv0.y });
		sbm::Matrix<2, 3> posMat({
			pos1.x - pos0.x, pos2.x - pos0.x,
			pos1.y - pos0.y, pos2.y - pos0.y,
			pos1.z - pos0.z, pos2.z - pos0.z,
			});
		auto tbMat = uvMat.GetInversed() * posMat;
		Vec3 tangent = tbMat.GetRow(0);
		Vec3 bitangent = tbMat.GetRow(1);
		Vec3 normal = Cross(bitangent, tangent);
		float w0 = sbm::Dot(normal, n0) < 0 ? -1 : 1;
		float w1 = Dot(normal, n1) < 0 ? -1 : 1;
		float w2 = Dot(normal, n2) < 0 ? -1 : 1;
		mesh->tangents.push_back((tangent - Dot(tangent, n0) * n0).Normalized().Expanded<4>(w0));
		mesh->tangents.push_back((tangent - Dot(tangent, n1) * n1).Normalized().Expanded<4>(w1));
		mesh->tangents.push_back((tangent - Dot(tangent, n2) * n2).Normalized().Expanded<4>(w2));
	}

	assert(mesh->tangents.size() == mesh->vertices.size());
}
