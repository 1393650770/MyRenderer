#include"VK_VertexBuffer.h"
#include"VK_Utils.h"
#include "../RenderUtils.h"
namespace MXRender
{

    
    VK_VertexBuffer::VK_VertexBuffer(const void* vertices, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE data_usage)
    {
        usage_type = data_usage;

    }

    VK_VertexBuffer::VK_VertexBuffer(std::shared_ptr<std::vector<float>> vertices, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE data_usage)
    {
        data = vertices;
        usage_type = data_usage;

        
    }

    VK_VertexBuffer::~VK_VertexBuffer()
    {

    }

    void VK_VertexBuffer::bind() const
    {

    }

    void VK_VertexBuffer::unbind() const
    {

    }

    void VK_VertexBuffer::set_alldata(const void* data, unsigned size)
    {

    }

    void VK_VertexBuffer::set_subdata(const void* data, unsigned offset, unsigned size)
    {

    }

    const Layout& VK_VertexBuffer::get_layout() const
    {
        return layout;
    }

    void VK_VertexBuffer::set_layout(const Layout& layout)
    {
        this->layout = layout;
    }


    
	std::vector<VkVertexInputAttributeDescription> VK_VertexBuffer::get_attribute_descriptions()
	{
        std::vector<VkVertexInputAttributeDescription> ret;
        ret.resize(layout.get_layout_element_size());
        int i=0;
        for (auto& it:layout)
        {
            VkFormat vulkan_data_type = VK_Utils::Translate_API_DataTypeEnum_To_Vulkan(it.data_type);
			unsigned vulkan_data_size = RenderUtils::Get_API_DataTypeEnum_To_OS_Size(it.data_type);
            ret[i].format= vulkan_data_type;
            ret[i].binding= i;
            ret[i].location=i;
            ret[i].offset= 0;
            i++;
        }
        return ret;
	}

	std::vector<VkVertexInputBindingDescription> VK_VertexBuffer::get_binding_bescriptions()
	{
		std::vector<VkVertexInputBindingDescription> ret;
		ret.resize(layout.get_layout_element_size());
		int i = 0;
		for (auto& it : layout)
		{
			VkFormat vulkan_data_type = VK_Utils::Translate_API_DataTypeEnum_To_Vulkan(it.data_type);
			unsigned vulkan_data_size = RenderUtils::Get_API_DataTypeEnum_To_OS_Size(it.data_type);

            ret[i].binding=i;
            ret[i].stride = vulkan_data_size;
            ret[i].inputRate= VK_VERTEX_INPUT_RATE_VERTEX;

			i++;
		}
		return ret;
	}

}