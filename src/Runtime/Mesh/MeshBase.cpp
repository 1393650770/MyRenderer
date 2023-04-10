#include "MeshBase.h"
#include "../RHI/Vulkan/VK_VertexArray.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include<unordered_map>
#include "../AssetLoader/texture_asset.h"
#include "../AssetLoader/mesh_asset.h"

MXRender::MeshBase::MeshBase()
{

}

MXRender::MeshBase::~MeshBase()
{

}

void MXRender::MeshBase::load_model(const std::string& filename)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str())) {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<SimpleVertex, uint32_t> uniqueVertices{};
	int i=0,f=0;
    for (const auto& shape : shapes) {
		
        for (const auto& index : shape.mesh.indices) {
            SimpleVertex vertex{};

            vertex.position = 
			{
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };
			vertex.uv=
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				attrib.texcoords[2 * index.texcoord_index + 1]
			};

			if (attrib.normals.size()>0)
			{
				glm::vec3 normal = glm::vec3(
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]);
				vertex.pack_normal(normal);
			}
			if (attrib.normals.size() > 0)
			{
				vertex.pack_color(glm::vec3{
					attrib.colors[3 * index.vertex_index + 0],
					attrib.colors[3 * index.vertex_index + 1],
					attrib.colors[3 * index.vertex_index + 2] });
			}

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
			f++;
        }
		i++;
    }
}

bool MXRender::MeshBase::load_asset(const  char* filename)
{
	assets::AssetFile file;
	bool loaded = assets::load_binaryfile(filename, file);

	if (!loaded) {
		std::cout << "Error when loading mesh " << filename << std::endl;;
		return false;
	}




	assets::MeshInfo meshinfo = assets::read_mesh_info(&file);



	std::vector<char> vertexBuffer;
	std::vector<char> indexBuffer;

	vertexBuffer.resize(meshinfo.vertexBuferSize);
	indexBuffer.resize(meshinfo.indexBuferSize);

	assets::unpack_mesh(&meshinfo, file.binaryBlob.data(), file.binaryBlob.size(), vertexBuffer.data(), indexBuffer.data());

	

	vertices.clear();
	indices.clear();

	indices.resize(indexBuffer.size() / sizeof(uint32_t));
	for (int i = 0; i < indices.size(); i++) {
		uint32_t* unpackedindices = (uint32_t*)indexBuffer.data();
		indices[i] = unpackedindices[i];
	}

	using namespace glm;
	if (meshinfo.vertexFormat == assets::VertexFormat::PNCV_F32)
	{
		assets::Vertex_f32_PNCV* unpackedVertices = (assets::Vertex_f32_PNCV*)vertexBuffer.data();

		vertices.resize(vertexBuffer.size() / sizeof(assets::Vertex_f32_PNCV));

		for (int i = 0; i < vertices.size(); i++) {

			vertices[i].position.x = unpackedVertices[i].position[0];
			vertices[i].position.y = unpackedVertices[i].position[1];
			vertices[i].position.z = unpackedVertices[i].position[2];

			vec3 normal = vec3(
				unpackedVertices[i].normal[0],
				unpackedVertices[i].normal[1],
				unpackedVertices[i].normal[2]);
			vertices[i].pack_normal(normal);

			vertices[i].pack_color(vec3{ unpackedVertices[i].color[0] ,unpackedVertices[i].color[1] ,unpackedVertices[i].color[2] });


			vertices[i].uv.x = unpackedVertices[i].uv[0];
			vertices[i].uv.y = unpackedVertices[i].uv[1];
		}
	}
	else if (meshinfo.vertexFormat == assets::VertexFormat::P32N8C8V16)
	{
		assets::Vertex_P32N8C8V16* unpackedVertices = (assets::Vertex_P32N8C8V16*)vertexBuffer.data();

		vertices.resize(vertexBuffer.size() / sizeof(assets::Vertex_P32N8C8V16));

		for (int i = 0; i < vertices.size(); i++) {

			vertices[i].position.x = unpackedVertices[i].position[0];
			vertices[i].position.y = unpackedVertices[i].position[1];
			vertices[i].position.z = unpackedVertices[i].position[2];

			vertices[i].pack_normal(vec3{
				 unpackedVertices[i].normal[0]
				,unpackedVertices[i].normal[1]
				,unpackedVertices[i].normal[2] });

			vertices[i].color.x = unpackedVertices[i].color[0];// / 255.f;
			vertices[i].color.y = unpackedVertices[i].color[1];// / 255.f;
			vertices[i].color.z = unpackedVertices[i].color[2];// / 255.f;

			vertices[i].uv.x = unpackedVertices[i].uv[0];
			vertices[i].uv.y = unpackedVertices[i].uv[1];
		}
	}


	is_prefabs=true;
	return true;
}

void MXRender::MeshBase::destroy_mesh_info(GraphicsContext* context)
{

}

void MXRender::MeshBase::init_mesh_info(GraphicsContext* context)
{

}
