#pragma once
#ifndef _GL_INDEXBUFFER_
#define _GL_INDEXBUFFER_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include"../RenderEnum.h"
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include"../IndexBuffer.h"
#include "vulkan/vulkan_core.h"

namespace MXRender { class VK_GraphicsContext; }

namespace MXRender { class VK_Device; }

namespace MXRender { class GraphicsContext; }
namespace MXRender
{
    class VK_IndexBuffer :public IndexBuffer
    {
    private:
        std::shared_ptr<std::vector<unsigned int>> index_data;
        unsigned id;
        unsigned index_data_size;
        ENUM_RENDER_DATA_USAGE_TYPE usage;
		
        VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
        std::weak_ptr<VK_Device> device;

    public:
        VK_IndexBuffer( const void* _index_array,unsigned size ,ENUM_RENDER_DATA_USAGE_TYPE data_usage);
        VK_IndexBuffer( std::shared_ptr<std::vector<unsigned int>> _index_array, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE data_usage);
		VK_IndexBuffer(std::shared_ptr<VK_GraphicsContext> context, std::shared_ptr<std::vector<unsigned int>> _index_array, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE data_usage);

        virtual ~VK_IndexBuffer();

        void bind() const override;
        void unbind() const override;

        void set_alldata(const void* _index_array, unsigned size) override;

        void set_subdata(const void* _index_array, unsigned offset, unsigned size) override;

        unsigned get_count() const override;

        void init(std::shared_ptr<VK_GraphicsContext> context, const void* _index_array, unsigned size);
    };
    
}
#endif
