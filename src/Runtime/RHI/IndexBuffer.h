#pragma once

#ifndef _INDEXBUFFER_
#define _INDEXBUFFER_
#include"RenderEnum.h"
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>

namespace MXRender
{
    class IndexBuffer
    {
    public:
        virtual ~IndexBuffer() = default;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        virtual void set_alldata(const void* _index_array, unsigned size) = 0;

        virtual void set_subdata(const void* _index_array, unsigned offset, unsigned size) = 0;


        virtual unsigned get_count() const = 0;

        static std::shared_ptr<IndexBuffer> CreateIndexBuffer(const void* _index_array, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE data_usage= ENUM_RENDER_DATA_USAGE_TYPE::DYNAMIC_DRAW);
        static std::shared_ptr<IndexBuffer> CreateIndexBuffer(std::shared_ptr<std::vector<unsigned int>> _index_array, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE data_usage = ENUM_RENDER_DATA_USAGE_TYPE::DYNAMIC_DRAW);

    };
}
#endif

