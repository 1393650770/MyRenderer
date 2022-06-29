#pragma once
#ifndef _VERTEXBUFFER_
#define _VERTEXBUFFER_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include "RenderEnum.h"

namespace MXRender
{
    struct Layout_Element
    {
    public:
        Layout_Element(ENUM_RENDER_DATA_TYPE _data_type, unsigned _num, ENUM_RENDER_ATTRIBUTE_TYPE::Enum _attributr_type, bool _normalized=false, bool _asInt=false) :
        data_type(_data_type),num(_num),attributr_type(_attributr_type),normalized(_normalized),asInt(_asInt)
        {};
        
        ENUM_RENDER_DATA_TYPE data_type;
        unsigned num;
        ENUM_RENDER_ATTRIBUTE_TYPE::Enum attributr_type;
        bool normalized = false;
        bool asInt = false;
    };

    class Layout
    {
    private:
        std::vector<Layout_Element> layout;
        unsigned stride;
        unsigned int offset[ENUM_RENDER_ATTRIBUTE_TYPE::Count];
    public:
        Layout()=default;
        Layout(const std::initializer_list<Layout_Element>& elements);
        virtual ~Layout();

        std::vector<Layout_Element>::iterator begin() { return layout.begin(); }
        std::vector<Layout_Element>::iterator end() { return layout.end(); }
        std::vector<Layout_Element>::const_iterator begin() const { return layout.cbegin(); }
        std::vector<Layout_Element>::const_iterator end() const { return layout.cend(); }
        const unsigned& get_stride() const;
        const unsigned int& get_offset(ENUM_RENDER_ATTRIBUTE_TYPE::Enum attribute_type) const;
    };


    class VertexBuffer
    {
    private:

    public:
        VertexBuffer() = default;
        virtual ~VertexBuffer();

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        virtual void set_alldata(const void* data, unsigned size) = 0;
        virtual void set_subdata(const void* data,unsigned offset, unsigned size) = 0;

        virtual const Layout& get_layout() const = 0;
        virtual void set_layout(const Layout& layout) = 0;

        static std::shared_ptr<VertexBuffer> CreateVertexBuffer(const void* vertices, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE usage = ENUM_RENDER_DATA_USAGE_TYPE::DYNAMIC_DRAW);

        static std::shared_ptr<VertexBuffer> CreateVertexBuffer(std::shared_ptr<std::vector<float>> vertices, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE usage = ENUM_RENDER_DATA_USAGE_TYPE::DYNAMIC_DRAW);

    };

}
#endif
