#include"VK_IndexBuffer.h"
#include "VK_GraphicsContext.h"


namespace MXRender
{
    VK_IndexBuffer::VK_IndexBuffer( const void* _index_array, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE data_usage)
    {
        usage = data_usage;
        index_data_size = size;
    }

    VK_IndexBuffer::VK_IndexBuffer( std::shared_ptr<std::vector<unsigned int>> _index_array, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE data_usage)
    {
        index_data = _index_array;
        usage = data_usage;
        index_data_size = size;

    }
    
	VK_IndexBuffer::VK_IndexBuffer(std::shared_ptr<VK_GraphicsContext> context, std::shared_ptr<std::vector<unsigned int>> _index_array, unsigned size, ENUM_RENDER_DATA_USAGE_TYPE data_usage)
	{
		index_data = _index_array;
		usage = data_usage;
		index_data_size = size;
        device=context->device;
        init(context,index_data->data(),index_data_size);
	}

	VK_IndexBuffer::~VK_IndexBuffer()
    {
        if (device.expired())
        {
            return;
        }
		vkDestroyBuffer(device.lock()->device, indexBuffer, nullptr);
		vkFreeMemory(device.lock()->device, indexBufferMemory, nullptr);
    }

    void VK_IndexBuffer::bind() const
    {

    }

    void VK_IndexBuffer::unbind() const
    {
 
    }

    void VK_IndexBuffer::set_alldata(const void* _index_array, unsigned size)
    {
        index_data_size = size;

    }

    void VK_IndexBuffer::set_subdata(const void* _index_array, unsigned offset, unsigned size)
    {

    }

    unsigned VK_IndexBuffer::get_count() const
    {
        return index_data_size/sizeof(unsigned int);
    }




    
	void VK_IndexBuffer::init(std::shared_ptr<VK_GraphicsContext> context, const void* _index_array, unsigned size)
	{
        if (device.expired())
        {
            return;
        }
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		VK_Utils::Create_VKBuffer(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device.lock()->device, stagingBufferMemory, 0, size, 0, &data);
		memcpy(data, _index_array, (size_t)size);
		vkUnmapMemory(device.lock()->device, stagingBufferMemory);

        VK_Utils::Create_VKBuffer(device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
        
        context->copy_buffer(stagingBuffer, indexBuffer, size);


		vkDestroyBuffer(device.lock()->device, stagingBuffer, nullptr);
		vkFreeMemory(device.lock()->device, stagingBufferMemory, nullptr);
	}

}