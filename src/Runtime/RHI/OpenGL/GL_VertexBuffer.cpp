#include"GL_VertexBuffer.h"
#include"GL_Utils.h"
namespace MXRender
{

    
    GL_VertexBuffer::GL_VertexBuffer(const void* vertices, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE data_usage)
    {
        usage_type = data_usage;
        glCreateBuffers(1, &id);
        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_Utils::Translate_API_UsageEnum_To_Opengl(usage_type));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GL_VertexBuffer::GL_VertexBuffer(std::shared_ptr<std::vector<float>> vertices, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE data_usage)
    {
        data = vertices;
        usage_type = data_usage;
        glCreateBuffers(1, &id);
        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferData(GL_ARRAY_BUFFER, size, data.get()->data(), GL_Utils::Translate_API_UsageEnum_To_Opengl(usage_type));
    }

    GL_VertexBuffer::~GL_VertexBuffer()
    {
        glDeleteBuffers(1, &id);
    }

    void GL_VertexBuffer::bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, id);
    }

    void GL_VertexBuffer::unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void GL_VertexBuffer::set_alldata(const void* data, unsigned size)
    {
        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_Utils::Translate_API_UsageEnum_To_Opengl(usage_type));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void GL_VertexBuffer::set_subdata(const void* data, unsigned offset, unsigned size)
    {
        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferSubData(GL_ARRAY_BUFFER ,offset, size, data );
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    const Layout& GL_VertexBuffer::get_layout() const
    {
        return layout;
    }

    void GL_VertexBuffer::set_layout(const Layout& layout)
    {
        this->layout = layout;
    }


    
}