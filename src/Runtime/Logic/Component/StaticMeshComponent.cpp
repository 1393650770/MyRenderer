#include "StaticMeshComponent.h"
#include <stdexcept>
#include "glm/ext/matrix_transform.hpp"

#include "../../Mesh/VK_Mesh.h"
#include "../../Mesh/GL_Mesh.h"
#include "../../RHI/RenderState.h"
#include "../../RHI/Vulkan/VK_GraphicsContext.h"
#include "../../Render/Pass/PipelineShaderObject.h"





void MXRender::StaticMeshComponent::spawn_mesh_data_shared_ptr()
{
	MeshBase* mesh_base = nullptr;
	switch (RenderState::render_api_type)
	{
	case ENUM_RENDER_API_TYPE::Vulkan:
	{
		mesh_base = new VK_Mesh();
		break;
	}
	case ENUM_RENDER_API_TYPE::OpenGL:
	{
		mesh_base=new GL_Mesh();
		break;
	}
	default:
		break;
	}
	mesh_data = mesh_base;
}



MXRender::StaticMeshComponent::StaticMeshComponent(const std::string& mesh_path)
{
	component_type=ComponentType::STATICMESH;
	spawn_mesh_data_shared_ptr();
	load_mesh( mesh_path);
}



MXRender::StaticMeshComponent::StaticMeshComponent()
{
	component_type = ComponentType::STATICMESH;
	spawn_mesh_data_shared_ptr();
}

MXRender::StaticMeshComponent::~StaticMeshComponent()
{
	//delete mesh_data;
}

void MXRender::StaticMeshComponent::load_mesh(const std::string& mesh_path)
{
	if(is_already_load_mesh)
	{
		throw std::logic_error("The mesh is already loaded.");
		return ;
	}
	mesh_data->load_model(mesh_path);
	//mesh_data->load_asset(mesh_path.c_str());
	set_already_load_mesh_to_true();
}

void MXRender::StaticMeshComponent::reset_mesh(MeshBase* mesh)
{
	delete mesh_data;
	mesh_data=mesh;
}

void MXRender::StaticMeshComponent::set_overmaterial(MXRender::Material* material)
{
	this->material=material;
}

 MXRender::Material* MXRender::StaticMeshComponent::get_material() const
{
	return material;
}

MXRender::MeshBase* MXRender::StaticMeshComponent::get_mesh_data()
{
	return mesh_data;
}

bool MXRender::StaticMeshComponent::get_already_load_mesh() const
{
	return is_already_load_mesh;
}

void MXRender::StaticMeshComponent::set_already_load_mesh_to_true()
{
	is_already_load_mesh=true;
}

void MXRender::StaticMeshComponent::render_mesh(RenderMeshInfo* render_mesh_info)
{
	switch (RenderState::render_api_type)
	{
	case ENUM_RENDER_API_TYPE::Vulkan:
	{
		VK_GraphicsContext* vk_context=dynamic_cast<VK_GraphicsContext*>(render_mesh_info->context);
		VK_Mesh* vk_mesh=dynamic_cast<VK_Mesh *>(mesh_data);
		if(!vk_context||!vk_mesh) return ;
		VkBuffer vertexBuffers[] = { vk_mesh->get_mesh_info().vertex_buffer };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(vk_context->get_cur_command_buffer(), 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(vk_context->get_cur_command_buffer(), vk_mesh->get_mesh_info().index_buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(vk_context->get_cur_command_buffer(), static_cast<uint32_t>(mesh_data->indices.size()), 1, 0, 0, 0);

		break;
	}
	default:
		break;
	}
}

void MXRender::StaticMeshComponent::bind_mesh(BindMeshInfo* bind_mesh_info)
{
	switch (RenderState::render_api_type)
	{
	case ENUM_RENDER_API_TYPE::Vulkan:
	{
		mesh_data->init_mesh_info(bind_mesh_info->context);
		break;
	}
	default:
		break;
	}
}

void MXRender::StaticMeshComponent::on_start()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void MXRender::StaticMeshComponent::on_update()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void MXRender::StaticMeshComponent::update(float delta_time)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void MXRender::StaticMeshComponent::on_end()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void MXRender::StaticMeshComponent::on_destroy()
{
	throw std::logic_error("The method or operation is not implemented.");
}

std::string MXRender::StaticMeshComponent::get_component_type_name()
{
	return "StaticMeshComponent";
}
