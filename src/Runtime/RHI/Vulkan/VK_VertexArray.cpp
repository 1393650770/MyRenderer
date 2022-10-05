
#include "VK_VertexArray.h"
#include"../VertexBuffer.h"
#include"../IndexBuffer.h"
#include"VK_Utils.h"
#include"../RenderUtils.h"
namespace MXRender
{

    VK_VertexArray::VK_VertexArray()
    {
        glGenVertexArrays(1, &id);
    }

    VK_VertexArray::~VK_VertexArray()
    {
        glDeleteVertexArrays(1, &id);
    }

    void VK_VertexArray::bind() const
    {
        glBindVertexArray(id);
    }

    void VK_VertexArray::unbind() const
    {
        glBindVertexArray(0);
    }

    void VK_VertexArray::set_vertexbuffer(const std::shared_ptr<VertexBuffer>& _vertex_buffer)
    {
        layout = _vertex_buffer;
        glBindVertexArray(id);
        
        auto& opengl_layout = layout->get_layout();
        unsigned index = 0;

       

    }

    void VK_VertexArray::set_indexbuffer(const std::shared_ptr<IndexBuffer>& _index_buffer)
    {

    }



}