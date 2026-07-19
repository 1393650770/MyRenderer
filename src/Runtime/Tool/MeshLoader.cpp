#include "MeshLoader.h"
#include <iostream>
#include <limits>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Tool)

Vector<RHI::VertexInputLayout> MeshDataPayload::GetVertexInputLayout()
{
	Vector<RHI::VertexInputLayout> layout(3);
	layout[0].binding = 0;
	layout[0].location = 0;
	layout[0].attribute_format = ENUM_TEXTURE_FORMAT::RGB32F;
	layout[0].offset = offsetof(MeshVertex, position);
	layout[1].binding = 0;
	layout[1].location = 1;
	layout[1].attribute_format = ENUM_TEXTURE_FORMAT::RGB32F;
	layout[1].offset = offsetof(MeshVertex, normal);
	layout[2].binding = 0;
	layout[2].location = 2;
	layout[2].attribute_format = ENUM_TEXTURE_FORMAT::RG32F;
	layout[2].offset = offsetof(MeshVertex, uv);
	return layout;
}

UInt32 MeshDataPayload::GetVertexStride()
{
	return (UInt32)sizeof(MeshVertex);
}

Bool MeshLoader::LoadMeshData(CONST String& in_filename, MeshDataPayload& out_payload)
{
	Assimp::Importer importer;
	CONST UInt32 flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals
		| aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices;
	CONST aiScene* scene = importer.ReadFile(in_filename, flags);
	if (scene == nullptr || !scene->HasMeshes())
	{
		std::cout << "[MeshLoader] failed to load: " << in_filename
			<< " (" << importer.GetErrorString() << ")" << std::endl;
		return false;
	}

	out_payload.vertices.clear();
	out_payload.indices.clear();
	out_payload.sub_meshes.clear();
	// parenthesized to dodge the windows.h min/max macros
	Float32 fmax = (std::numeric_limits<Float32>::max)();
	Float32 bmin[3] = { fmax, fmax, fmax };
	Float32 bmax[3] = { -fmax, -fmax, -fmax };

	for (UInt32 mesh_index = 0; mesh_index < scene->mNumMeshes; ++mesh_index)
	{
		CONST aiMesh* mesh = scene->mMeshes[mesh_index];
		UInt32 vertex_base = (UInt32)out_payload.vertices.size();

		MeshDataPayload::SubMesh sub;
		sub.index_offset = (UInt32)out_payload.indices.size();
		sub.index_count = mesh->mNumFaces * 3;
		out_payload.sub_meshes.push_back(sub);

		for (UInt32 v = 0; v < mesh->mNumVertices; ++v)
		{
			MeshVertex vertex;
			vertex.position[0] = mesh->mVertices[v].x;
			vertex.position[1] = mesh->mVertices[v].y;
			vertex.position[2] = mesh->mVertices[v].z;
			if (mesh->HasNormals())
			{
				vertex.normal[0] = mesh->mNormals[v].x;
				vertex.normal[1] = mesh->mNormals[v].y;
				vertex.normal[2] = mesh->mNormals[v].z;
			}
			if (mesh->HasTextureCoords(0))
			{
				vertex.uv[0] = mesh->mTextureCoords[0][v].x;
				vertex.uv[1] = mesh->mTextureCoords[0][v].y;
			}
			for (Int axis = 0; axis < 3; ++axis)
			{
				if (vertex.position[axis] < bmin[axis]) bmin[axis] = vertex.position[axis];
				if (vertex.position[axis] > bmax[axis]) bmax[axis] = vertex.position[axis];
			}
			out_payload.vertices.push_back(vertex);
		}

		for (UInt32 f = 0; f < mesh->mNumFaces; ++f)
		{
			CONST aiFace& face = mesh->mFaces[f];
			// aiProcess_Triangulate guarantees 3 indices per face
			out_payload.indices.push_back(vertex_base + face.mIndices[0]);
			out_payload.indices.push_back(vertex_base + face.mIndices[1]);
			out_payload.indices.push_back(vertex_base + face.mIndices[2]);
		}
	}

	for (Int axis = 0; axis < 3; ++axis)
	{
		out_payload.bounds_min[axis] = bmin[axis];
		out_payload.bounds_max[axis] = bmax[axis];
	}
	return !out_payload.vertices.empty() && !out_payload.indices.empty();
}

void MeshLoader::LoadMeshData(CONST String& in_filename, MeshDataPayload* out_payload, std::atomic_bool& out_result)
{
	if (out_payload == nullptr)
		return;
	Bool ok = LoadMeshData(in_filename, *out_payload);
	out_result = ok;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
