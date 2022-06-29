#include"GL_IndexBuffer.h"
#include"GL_Utils.h"
namespace MXRender
{
    GL_IndexBuffer::GL_IndexBuffer(const void* _index_array, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE data_usage)
    {
        usage = data_usage;
        index_data_size = size;
        glCreateBuffers(1, &id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, _index_array, GL_Utils::Translate_API_UsageEnum_To_Opengl(usage));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    GL_IndexBuffer::GL_IndexBuffer(std::shared_ptr<std::vector<unsigned int>> _index_array, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE data_usage)
    {
        index_data = _index_array;
        usage = data_usage;
        index_data_size = size;
        glCreateBuffers(1, &id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data_size, index_data.get()->data(), GL_Utils::Translate_API_UsageEnum_To_Opengl(usage));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    
    GL_IndexBuffer::~GL_IndexBuffer()
    {
        glDeleteBuffers(1, &id);
    }

    void GL_IndexBuffer::bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    }

    void GL_IndexBuffer::unbind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void GL_IndexBuffer::set_alldata(const void* _index_array, unsigned size)
    {
        index_data_size = size;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, _index_array, GL_Utils::Translate_API_UsageEnum_To_Opengl(usage));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void GL_IndexBuffer::set_subdata(const void* _index_array, unsigned offset, unsigned size)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
        glBufferSubData (GL_ELEMENT_ARRAY_BUFFER, offset,size, _index_array);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    unsigned GL_IndexBuffer::get_count() const
    {
        return index_data_size/sizeof(unsigned int);
    }




    
}