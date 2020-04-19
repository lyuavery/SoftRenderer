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
			mesh = LoadObj(fileName, bGenerateNormals);
		}
		else {
			XLogWarning("Unsopported file ext: %s", ext.c_str());
		}
	} while (false);
	
	if (mesh)
	{
		if (bGenerateNormals)
		{
			mesh->EnableAttribute(true, VertexAttribute::Normals);
			GenerateNormals(mesh);
		}
		if (bGenerateTangents)
		{
			mesh->EnableAttribute(true, VertexAttribute::Tangents);
			GenerateTangents(mesh);
		}
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
	bool hasVertices = false;
	bool hasIndices = false;

	auto& indices = mesh->indices;
	auto& vertices = mesh->vertices;
	auto& normals = mesh->normals;
	auto& uvs = mesh->uvs;
	bool bRemap = false;
	const UInt32 limit = (1 << 16) - 1;
	std::unordered_map<UInt64, Vec4ui> props;
	UInt32 propCnt = 0;
	std::vector<UInt64> propsRef;
	while (!in.eof()) {
		std::getline(in, line);
		std::istringstream iss(line);
		char trash;
		if (!line.compare(0, 2, "v ")) {
			hasVertices = true;
			iss >> trash;
			Vec3 v;
			for (int i = 0; i < 3; i++) iss >> v[i];
			vertices.push_back(v);
		}
		else if (!line.compare(0, 3, "vn ")) {
			if (bGenerateNormals) continue;
			mesh->EnableAttribute(true, VertexAttribute::Normals);
			iss >> trash >> trash;
			Vec3 n;
			for (int i = 0; i < 3; i++) iss >> n[i];
			normals.push_back(n);
		}
		else if (!line.compare(0, 3, "vt ")) {
			mesh->EnableAttribute(true, VertexAttribute::UVs);
			iss >> trash >> trash;
			Vec2 uv;
			for (int i = 0; i < 2; i++) iss >> uv[i];
			uvs.push_back(uv);
		}
		else if (!line.compare(0, 2, "f ")) {  // 索引范围暂时只支持到[0,2^16-1]
			hasIndices = true;
			int v0,v1,v2;
			iss >> trash;
			while (iss >> v0 >> trash >> v1 >> trash >> v2) {
				--v0; --v1; --v2;
				if (v0 != v1 || v0 != v2 || v1 != v2) bRemap = true;
				assert(v0 < limit && v1 < limit && v2 < limit);

				UInt64 key = (UInt64(v0) << 48) + (UInt64(v1) << 32) + (bGenerateNormals ? 0 : (UInt64(v2) << 16));
				if (props.find(key) == props.end()) props[key] = Vec4ui(v0, v1, v2, propCnt++);
				propsRef.push_back(key);
				indices.push_back(v0);
			}
		}
	}
	if (!(hasVertices && hasIndices))
	{
		XLogWarning("Obj file has no vertices or indices: %s.\n", fileName.c_str());
		return nullptr;
	}

	// 根据索引调整buffer
	if (bRemap)
	{
		auto maxsize = props.size();
		std::vector<UInt32> idxRemap(indices.size(), -1);
		std::vector<Vec3> verRemap(maxsize, Vec3(0));
		std::vector<Vec2> uvRemap(maxsize, Vec2(0));
		std::vector<Vec3> nRemap(bGenerateNormals ? 0 : maxsize, Vec3(0));
		
		for (int i = 0, n = propsRef.size(); i < n; ++i)
		{
			auto& vec = props[propsRef[i]];
			idxRemap[i] = vec.w;
			verRemap[vec.w] = vertices[vec.x];
			uvRemap[vec.w] = uvs[vec.y];
			if(!bGenerateNormals) nRemap[vec.w] = normals[vec.z];
		}
		vertices.swap(verRemap);
		uvs.swap(uvRemap);
		if (!bGenerateNormals) normals.swap(nRemap);
		indices.swap(idxRemap);
	}
	
	XLogInfo("Load mesh successfully: %s, v# %d, f# %d, vt# %d, vn# %d\n", fileName.c_str(), mesh->vertices.size(), mesh->indices.size(), mesh->uvs.size(), mesh->normals.size());
	return mesh;
}

void SR::MeshLoader::GenerateNormals(Mesh* mesh) // TODO
{
	XLogWarning("TODO: GenerateNormals ");
}

void SR::MeshLoader::GenerateTangents(SR::Mesh* mesh)
{
	
	if (!mesh || mesh->vertices.empty() || mesh->uvs.empty() || mesh->normals.empty()) return;
	auto& vertices = mesh->vertices;
	auto& uvs = mesh->uvs;
	auto& normals = mesh->normals;

	const auto& f = mesh->indices;
	int vSize = mesh->vertices.size();
	std::vector<Vec4> tangents(vSize, Vec4(0));
	std::vector<Vec3> bitangents(vSize, Vec3(0));

	for (int i = 0, n = mesh->indices.size(); i < n; i += 3)
	{
		UInt32 id0 = f[i], id1 = f[i + 1], id2 = f[i + 2];
		const auto& pos0 = vertices[id0];
		const auto& pos1 = vertices[id1];
		const auto& pos2 = vertices[id2];

		const auto& uv0 = uvs[id0];
		const auto& uv1 = uvs[id1];
		const auto& uv2 = uvs[id2];

		const auto& n0 = normals[id0];
		const auto& n1 = normals[id1];
		const auto& n2 = normals[id2];

		sbm::Matrix<2, 2> uvMat({ uv1.x - uv0.x, uv2.x - uv0.x,uv1.y - uv0.y, uv2.y - uv0.y });
		sbm::Matrix<2, 3> posMat({
			pos1.x - pos0.x, pos2.x - pos0.x,
			pos1.y - pos0.y, pos2.y - pos0.y,
			pos1.z - pos0.z, pos2.z - pos0.z,
			});
		auto tbMat = uvMat.GetInversed() * posMat;
		Vec3 tangent = tbMat.GetRow(0);
		Vec3 bitangent = tbMat.GetRow(1);
		//Vec3 normal = Cross(bitangent, tangent);
		tangents[id0] += tangent.Normalized().Expanded<4>(0);
		tangents[id1] += tangent.Normalized().Expanded<4>(0);
		tangents[id2] += tangent.Normalized().Expanded<4>(0);
		bitangents[id0] += bitangent.Normalized();
		bitangents[id1] += bitangent.Normalized();
		bitangents[id2] += bitangent.Normalized();
	/*	float w0 = sbm::Dot(normal, n0) < 0 ? -1 : 1;
		float w1 = Dot(normal, n1) < 0 ? -1 : 1;
		float w2 = Dot(normal, n2) < 0 ? -1 : 1;*/
	/*	tangents.push_back((tangent - Dot(tangent, n0) * n0).Normalized().Expanded<4>(w0));
		tangents.push_back((tangent - Dot(tangent, n1) * n1).Normalized().Expanded<4>(w1));
		tangents.push_back((tangent - Dot(tangent, n2) * n2).Normalized().Expanded<4>(w2));*/

	/*	tangents.push_back((tangent ).Normalized().Expanded<4>(w0));
		tangents.push_back((tangent ).Normalized().Expanded<4>(w1));
		tangents.push_back((tangent ).Normalized().Expanded<4>(w2));*/
	//https://www.marti.works/calculating-tangents-for-your-mesh/
		/*tangents[id0] = tangents[id0] + (tangent - Dot(tangent, n0) * n0).Normalized().Expanded<4>(w0);
		tangents[id1] = tangents[id1] + (tangent - Dot(tangent, n1) * n1).Normalized().Expanded<4>(w1);
		tangents[id2] = tangents[id2] + (tangent - Dot(tangent, n2) * n2).Normalized().Expanded<4>(w2);*/
	}

	for (int i = 0, n = vSize; i < n; ++i)
	{
		Vec3 tangent = tangents[i].Normalized().Truncated<3>();
		bitangents[i].Normalize();
		Vec3 normal = normals[i].Normalized();

		tangent = (tangent - sbm::Dot(tangent, normal) * normal).Normalized();
		float w = (sbm::Dot(bitangents[i], sbm::Cross(normal, tangent)) < 0) ? -1 : 1;
		//assert(!sbm::is_special_float(w));
		tangents[i] = tangent.Expanded<4>(w);
	}
	mesh->tangents.swap(tangents);
}
