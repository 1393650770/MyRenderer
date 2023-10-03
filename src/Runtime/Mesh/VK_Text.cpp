#include "VK_Text.h"
#include "../RHI/Vulkan/VK_GraphicsContext.h"
#include "../Tools/Font/FontTools.h"
#include "../Tools/Font/save-png.h"
#include "../Render/DefaultSetting.h"
#include "../Utils/Singleton.h"



void MXRender::VK_Text::setup_vk_vertexbuffer(VK_GraphicsContext* cur_context)
{
	float base_x=0.0f,base_y=0.0f;
	for (auto& it : content)
	{
		std::vector<SimpleVertex> vertices;
		SimpleVertex vertice[4];
		auto& ch = vk_text_info.Characters[it];
		float xpos = base_x + ch.bearing.x ;
		float ypos = base_y - (ch.size.y - ch.bearing.y) ;
		float w = ch.size.x ;
		float h = ch.size.y ;
		vertice[0].position = glm::vec3(xpos, ypos + h,0.0f);
		vertice[0].uv = glm::vec2(0.0f,0.0f);
		vertice[1].position = glm::vec3(xpos, ypos, 0.0f);
		vertice[1].uv = glm::vec2(0.0f, 1.0f);
		vertice[2].position = glm::vec3(xpos + w, ypos , 0.0f);
		vertice[2].uv = glm::vec2(1.0f, 1.0f);
		vertice[3].position = glm::vec3(xpos + w, ypos + h, 0.0f);
		vertice[3].uv = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertice[0]);
		vertices.push_back(vertice[1]);
		vertices.push_back(vertice[2]);
		vertices.push_back(vertice[3]);

		base_x += w;

		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		VK_Utils::Create_VKBuffer(cur_context->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(cur_context->device->device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);

		vkUnmapMemory(cur_context->device->device, stagingBufferMemory);


		VK_Utils::Create_VKBuffer(cur_context->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vk_text_info.MeshInfos[it].vertex_buffer, vk_text_info.MeshInfos[it].vertex_buffer_memory);

		VK_Utils::Copy_VKBuffer(cur_context, stagingBuffer, vk_text_info.MeshInfos[it].vertex_buffer, bufferSize);

		vkDestroyBuffer(cur_context->device->device, stagingBuffer, nullptr);
		vkFreeMemory(cur_context->device->device, stagingBufferMemory, nullptr);
	}
}

void MXRender::VK_Text::setup_vk_indexbuffer(VK_GraphicsContext* cur_context)
{
	for (auto& it : content)
	{
		std::vector<uint32_t> indices=
		{
			0, 1, 3,
			1, 2, 3
		};

		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		VK_Utils::Create_VKBuffer(cur_context->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(cur_context->device->device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(cur_context->device->device, stagingBufferMemory);


		VK_Utils::Create_VKBuffer(cur_context->device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vk_text_info.MeshInfos[it].index_buffer, vk_text_info.MeshInfos[it].index_buffer_memory);

		VK_Utils::Copy_VKBuffer(cur_context, stagingBuffer, vk_text_info.MeshInfos[it].index_buffer, bufferSize);

		vkDestroyBuffer(cur_context->device->device, stagingBuffer, nullptr);
		vkFreeMemory(cur_context->device->device, stagingBufferMemory, nullptr);
	}
}

MXRender::VK_Text::VK_Text()
{

}

MXRender::VK_Text::~VK_Text()
{

}

void MXRender::VK_Text::load_ttf(const std::string& ttf_path)
{
	if (is_already_init_ttf)
	{
		return;
	}
	msdfgen::FreetypeHandle* ft = msdfgen::FreetypeHandle::initializeFreetype();
	if (ft) {
		msdfgen::FontHandle* font = msdfgen::FontHandle::loadFont(ft, ttf_path.c_str());
		if (font) 
		{
			glm::vec2 size(64.0f, 64.0f);
			for (GLubyte c = 0; c < 128; c++)
			{
				float font_size = (font->face->units_per_EM / 64) * 5.5f;
				msdfgen::Bitmap<float, 1>* sdf_bitmap = FontTools::GenerateSdf(*font, font_size, c, size, 3.f);
				if(!sdf_bitmap) 
					continue;
				//std::string save_png_path = "SDFText" + std::to_string(c) + ".png";
				//msdfgen::savePng(*sdf_bitmap, save_png_path.c_str());
				vk_text_info.Characters[c].text_texture = new VK_Texture();
				FontTools::TranslateBitmapToVkTexture(vk_text_info.Characters[c].text_texture, *sdf_bitmap);
				vk_text_info.Characters[c].size = glm::vec2(sdf_bitmap->width(), sdf_bitmap->height());//glm::ivec2(sdf_bitmap->width(), sdf_bitmap->height());//glm::ivec2(font_size, font_size);
				vk_text_info.Characters[c].bearing = glm::ivec2(font->face->glyph->bitmap_left, font->face->glyph->bitmap_top);//(font_size, font_size);//(font->face->glyph->bitmap_left, font->face->glyph->bitmap_top);
				delete sdf_bitmap;
			}
			msdfgen::FontHandle::destroyFont(font);
		}
		msdfgen::FreetypeHandle::deinitializeFreetype(ft);
		is_already_init_ttf = true;
	}

}

void MXRender::VK_Text::destroy_text_info(GraphicsContext* context)
{
	for(auto& it : vk_text_info.Characters)
	{ 
		if (it.second.text_texture)
		{

			delete it.second.text_texture;
			vk_text_info.Characters[it.first].text_texture = nullptr;
		}
	}
	vk_text_info.Characters.clear();
	VK_GraphicsContext* vk_context = dynamic_cast<VK_GraphicsContext*>(context);
	if (!vk_context) return;
	for (auto& it : vk_text_info.MeshInfos)
	{
		if (it.second.vertex_buffer != VK_NULL_HANDLE || it.second.vertex_buffer_memory != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(vk_context->device->device, it.second.vertex_buffer, nullptr);
			vkFreeMemory(vk_context->device->device, it.second.vertex_buffer_memory, nullptr);
			it.second.vertex_buffer_memory = VK_NULL_HANDLE;
			it.second.vertex_buffer = VK_NULL_HANDLE;
		}
		if (it.second.index_buffer != VK_NULL_HANDLE || it.second.index_buffer_memory != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(vk_context->device->device, it.second.index_buffer, nullptr);
			vkFreeMemory(vk_context->device->device, it.second.index_buffer_memory, nullptr);
			it.second.index_buffer_memory = VK_NULL_HANDLE;
			it.second.index_buffer = VK_NULL_HANDLE;
		}
	}
	vk_text_info.MeshInfos.clear();
}

void MXRender::VK_Text::init_text_info(GraphicsContext* context)
{
	if (is_need_to_update)
	{
		VK_GraphicsContext* vk_context = dynamic_cast<VK_GraphicsContext*>(context);
		if (!vk_context) return;
		for (auto& it : vk_text_info.MeshInfos)
		{
			if (it.second.vertex_buffer != VK_NULL_HANDLE || it.second.vertex_buffer_memory != VK_NULL_HANDLE)
			{
				vkDestroyBuffer(vk_context->device->device, it.second.vertex_buffer, nullptr);
				vkFreeMemory(vk_context->device->device, it.second.vertex_buffer_memory, nullptr);
				it.second.vertex_buffer_memory = VK_NULL_HANDLE;
				it.second.vertex_buffer = VK_NULL_HANDLE;
			}
			if (it.second.index_buffer != VK_NULL_HANDLE || it.second.index_buffer_memory != VK_NULL_HANDLE)
			{
				vkDestroyBuffer(vk_context->device->device, it.second.index_buffer, nullptr);
				vkFreeMemory(vk_context->device->device, it.second.index_buffer_memory, nullptr);
				it.second.index_buffer_memory = VK_NULL_HANDLE;
				it.second.index_buffer = VK_NULL_HANDLE;
			}
		}
		setup_vk_vertexbuffer(vk_context);
		setup_vk_indexbuffer(vk_context);
	}
}

MXRender::VK_TextInfo& MXRender::VK_Text::get_text_info()
{
	return vk_text_info;
}

void MXRender::VK_Text::bind(GraphicsContext* context)
{
	init_text_info(context);
}

void MXRender::VK_Text::render(GraphicsContext* context)
{
	VK_GraphicsContext* vk_context = dynamic_cast<VK_GraphicsContext*>(context);;
	if (!vk_context) return;
	std::string content = get_content();
	for (char& it : content)
	{
		char key = it;
		Material* sdf_textMaterial=nullptr;
		MaterialData info;
		info.parameters = nullptr;
		info.textures.clear();
		info.baseTemplate = "sdf_text";
		sdf_textMaterial = Singleton<DefaultSetting>::get_instance().material_system->build_material("sdf_text", info);

		VkBuffer vertexBuffers[] = { get_text_info().MeshInfos[key].vertex_buffer };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(vk_context->get_cur_command_buffer(), 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(vk_context->get_cur_command_buffer(),get_text_info().MeshInfos[key].index_buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(vk_context->get_cur_command_buffer(), 6, 1, 0, 0, 0);
	}
}

