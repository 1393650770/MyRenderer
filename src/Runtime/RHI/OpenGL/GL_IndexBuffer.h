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
namespace MXRender
{
    class GL_IndexBuffer :public IndexBuffer
    {
    private:
        std::shared_ptr<std::vector<unsigned int>> index_data;
        unsigned id;
        unsigned index_data_size;
        ENUM_RENDER_DATA_USAGE_TYPE usage;
    public:
        GL_IndexBuffer(const void* _index_array,unsigned size ,ENUM_RENDER_DATA_USAGE_TYPE data_usage);
        GL_IndexBuffer(std::shared_ptr<std::vector<unsigned int>> _index_array, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE data_usage);

        virtual ~GL_IndexBuffer();

        void bind() const override;
        void unbind() const override;

        void set_alldata(const void* _index_array, unsigned size) override;

        void set_subdata(const void* _index_array, unsigned offset, unsigned size) override;

        unsigned get_count() const override;

    };
    
}
#endif
