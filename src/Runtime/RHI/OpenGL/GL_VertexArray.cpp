
#include "GL_VertexArray.h"
#include"../VertexBuffer.h"
#include"../IndexBuffer.h"
#include"GL_Utils.h"
#include"../RenderUtils.h"
namespace MXRender
{

    GL_VertexArray::GL_VertexArray()
    {
        glGenVertexArrays(1, &id);
    }

    GL_VertexArray::~GL_VertexArray()
    {
        glDeleteVertexArrays(1, &id);
    }

    void GL_VertexArray::bind() const
    {
        glBindVertexArray(id);
    }

    void GL_VertexArray::unbind() const
    {
        glBindVertexArray(0);
    }

    void GL_VertexArray::set_vertexbuffer(const std::shared_ptr<VertexBuffer>& _vertex_buffer)
    {
        layout = _vertex_buffer;
        glBindVertexArray(id);
        layout->bind();
        auto& opengl_layout = layout->get_layout();
        unsigned index = 0;

        for (auto& it : opengl_layout )
        {
            GLenum opengl_data_type= GL_Utils::Translate_API_DataTypeEnum_To_Opengl(it.data_type);
            unsigned opengl_data_size = RenderUtils::Get_API_DataTypeEnum_To_OS_Size(it.data_type);
            switch (it.data_type)
            {
            case MXRender::ENUM_RENDER_DATA_TYPE::Float:
            case MXRender::ENUM_RENDER_DATA_TYPE::Half:
            {
                glEnableVertexAttribArray(index);
                glVertexAttribPointer(index, 
                    it.num, 
                    opengl_data_type, 
                    it.normalized ? GL_TRUE : GL_FALSE, 
                    opengl_layout.get_stride() * opengl_data_size,
                    (void* )(opengl_layout.get_offset(it.attributr_type) * opengl_data_size)
                );

                std::cout <<index<<"---" << it.num << "---" << opengl_layout.get_stride() * opengl_data_size <<  "----" << opengl_layout.get_offset(it.attributr_type) <<"---" << opengl_data_size <<"---"<< opengl_data_type << std::endl;
                break;
            }
            case MXRender::ENUM_RENDER_DATA_TYPE::Int:
            case MXRender::ENUM_RENDER_DATA_TYPE::Uint8:
            case MXRender::ENUM_RENDER_DATA_TYPE::Uint10:
            case MXRender::ENUM_RENDER_DATA_TYPE::Int16:
            case MXRender::ENUM_RENDER_DATA_TYPE::Bool:
            {
                glEnableVertexAttribArray(index);
                glVertexAttribIPointer(index,
                    it.num, opengl_data_type,
                    opengl_layout.get_stride() * opengl_data_size,
                    (void*)(opengl_layout.get_offset(it.attributr_type) * opengl_data_size)
                );
                
                break;
            }
            default:
                index--;
                break;

            }
            index++;
        }


    }

    void GL_VertexArray::set_indexbuffer(const std::shared_ptr<IndexBuffer>& _index_buffer)
    {
        bind();
        _index_buffer->bind();
        ebo = _index_buffer;
    }



}