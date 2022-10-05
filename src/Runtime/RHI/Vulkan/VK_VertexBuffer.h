#pragma once
#ifndef _VK_VERTEXBUFFER_
#define _VK_VERTEXBUFFER_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include"../RenderEnum.h"
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include"../VertexBuffer.h"
#include "vulkan/vulkan_core.h"

namespace MXRender
{
    class VK_VertexBuffer:public VertexBuffer
    {
    private:
        std::shared_ptr<std::vector<float>> data;
        unsigned id;
        Layout layout;
        ENUM_RENDER_DATA_USAGE_TYPE usage_type;
    public:
        VK_VertexBuffer(const void* vertices, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE data_usage);
        VK_VertexBuffer(std::shared_ptr<std::vector<float>> vertices, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE data_usage);
        virtual ~VK_VertexBuffer();

        void bind() const override;
        void unbind() const override;

        void set_alldata(const void* data, unsigned size) override;
        void set_subdata(const void* data, unsigned offset, unsigned size) override;

        const Layout& get_layout() const override;
        void set_layout(const Layout& layout) override;


        std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();

        std::vector<VkVertexInputBindingDescription> get_binding_bescriptions();
    };
    
}
#endif
