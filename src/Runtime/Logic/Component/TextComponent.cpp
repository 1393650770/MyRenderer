#include "TextComponent.h"
#include "../../RHI/RenderState.h"
#include "../../Mesh/VK_Text.h"
#include "../../RHI/Vulkan/VK_GraphicsContext.h"
#include "../../Mesh/MeshBase.h"


namespace MXRender
{

	TextComponent::TextComponent()
	{

	}

	TextComponent::TextComponent(const std::string& ttf_path)
	{
		TextBase* new_text_data = nullptr;
		switch (RenderState::render_api_type)
		{
		case ENUM_RENDER_API_TYPE::Vulkan:
		{
			new_text_data = new VK_Text();
			break;
		}
		default:
			break;
		}
		text_data = new_text_data;
		text_data->load_ttf(ttf_path);
	}

	TextComponent::~TextComponent()
	{

	}

	void TextComponent::on_start()
	{

	}

	void TextComponent::on_update()
	{
		
	}

	void TextComponent::update(float delta_time)
	{

	}

	void TextComponent::on_end()
	{

	}

	void TextComponent::on_destroy()
	{

	}

	std::string TextComponent::get_component_type_name()
	{
		return "TextComponent";
	}

	void TextComponent::reset_text(TextBase* in_text_data)
	{
		delete text_data;
		text_data=in_text_data;
	}

	void TextComponent::reset_text_content(const std::string& in_text_content)
	{
		text_content= in_text_content;
		if (text_data != nullptr)
		{
			text_data->update_content(in_text_content);
		}
	}

	void TextComponent::render_text(RenderInfo* render_mesh_info)
	{
		switch (RenderState::render_api_type)
		{
		case ENUM_RENDER_API_TYPE::Vulkan:
		{
			VK_GraphicsContext* vk_context = dynamic_cast<VK_GraphicsContext*>(render_mesh_info->context);
			VK_Text* vk_text = dynamic_cast<VK_Text*>(text_data);
			if (!vk_context || !vk_text) return;
			std::string content = vk_text->get_content();
			for(char& it : content)
			{ 
				char key =it;
				VkBuffer vertexBuffers[] = { vk_text->get_text_info().MeshInfos[key].vertex_buffer };
				VkDeviceSize offsets[] = { 0 };

				vkCmdBindVertexBuffers(vk_context->get_cur_command_buffer(), 0, 1, vertexBuffers, offsets);

				vkCmdBindIndexBuffer(vk_context->get_cur_command_buffer(), vk_text->get_text_info().MeshInfos[key].index_buffer, 0, VK_INDEX_TYPE_UINT32);

				vkCmdDrawIndexed(vk_context->get_cur_command_buffer(),6, 1, 0, 0, 0);
			}
			break;
		}
		default:
			break;
		}
	}

	void TextComponent::bind_text(BindInfo* bind_mesh_info)
	{
		switch (RenderState::render_api_type)
		{
		case ENUM_RENDER_API_TYPE::Vulkan:
		{
			text_data->init_text_info(bind_mesh_info->context);
			break;
		}
		default:
			break;
		}
	}

	MXRender::TextBase* TextComponent::get_text_data()
	{
		return text_data;
	}	

}