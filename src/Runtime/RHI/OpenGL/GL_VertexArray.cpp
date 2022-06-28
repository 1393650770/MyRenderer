
#include "GL_VertexArray.h"
#include"../VertexBuffer.h"
#include"../IndexBuffer.h"

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

    }

    void GL_VertexArray::set_indexbuffer(const std::shared_ptr<IndexBuffer>& _index_buffer)
    {
        bind();
        _index_buffer->bind();
        ebo = _index_buffer;
    }



}