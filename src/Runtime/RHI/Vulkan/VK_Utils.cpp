

#include "VK_Utils.h"
namespace MXRender
{
    uint32_t findMemoryType(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties_flag)
    {
		VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
		vkGetPhysicalDeviceMemoryProperties(physical_device, &physical_device_memory_properties);
		for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
		{
			if (type_filter & (1 << i) &&
				(physical_device_memory_properties.memoryTypes[i].propertyFlags & properties_flag) == properties_flag)
			{
				return i;
			}
		}
		throw std::runtime_error("findMemoryType");
    }

    void VK_Utils::Create_VKBuffer(std::weak_ptr<VK_Device> Device, VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, VkBuffer& Buffer, VkDeviceMemory& BufferMemory)
    {
        if (Device.expired())
        {
            return ;
        }
        std::shared_ptr<VK_Device> DeviceSharedPtr=Device.lock();
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = Size;
        bufferInfo.usage = Usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(DeviceSharedPtr->device, &bufferInfo, nullptr, &Buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(DeviceSharedPtr->device, Buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = Find_MemoryType(Device,memRequirements.memoryTypeBits, Properties);

        if (vkAllocateMemory(DeviceSharedPtr->device, &allocInfo, nullptr, &BufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(DeviceSharedPtr->device, Buffer, BufferMemory, 0);
    }



	void VK_Utils::Create_Image(VkPhysicalDevice physical_device, VkDevice device, uint32_t image_width, uint32_t image_height, VkFormat format, VkImageTiling image_tiling, VkImageUsageFlags image_usage_flags, VkMemoryPropertyFlags memory_property_flags, VkImage& image, VkDeviceMemory& memory, VkImageCreateFlags image_create_flags, uint32_t array_layers, uint32_t miplevels)
	{
		VkImageCreateInfo image_create_info{};
		image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.flags = image_create_flags;
		image_create_info.imageType = VK_IMAGE_TYPE_2D;
		image_create_info.extent.width = image_width;
		image_create_info.extent.height = image_height;
		image_create_info.extent.depth = 1;
		image_create_info.mipLevels = miplevels;
		image_create_info.arrayLayers = array_layers;
		image_create_info.format = format;
		image_create_info.tiling = image_tiling;
		image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_create_info.usage = image_usage_flags;
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(device, &image_create_info, nullptr, &image) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex =
			findMemoryType(physical_device, memRequirements.memoryTypeBits, memory_property_flags);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device, image, memory, 0);
	}

	VkImageView VK_Utils::Create_ImageView(VkDevice device, VkImage& image, VkFormat format, VkImageAspectFlags image_aspect_flags, VkImageViewType view_type, uint32_t layout_count, uint32_t miplevels)
	{
		VkImageViewCreateInfo image_view_create_info{};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.image = image;
		image_view_create_info.viewType = view_type;
		image_view_create_info.format = format;
		image_view_create_info.subresourceRange.aspectMask = image_aspect_flags;
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.levelCount = miplevels;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount = layout_count;

		VkImageView image_view;
		if (vkCreateImageView(device, &image_view_create_info, nullptr, &image_view) != VK_SUCCESS)
		{
			return image_view;
			// todo
		}

		return image_view;
	}

	uint32_t VK_Utils::Find_MemoryType(std::weak_ptr<VK_Device> Device, uint32_t TypeFilter, VkMemoryPropertyFlags Properties)
    {
        if (Device.expired())
        {
            return -1;
        }
        std::shared_ptr<VK_Device> DeviceSharedPtr = Device.lock();

        VkPhysicalDeviceMemoryProperties MemProperties;
        vkGetPhysicalDeviceMemoryProperties(DeviceSharedPtr->gpu, &MemProperties);

        for (uint32_t i = 0; i < MemProperties.memoryTypeCount; i++) {
            if ((TypeFilter & (1 << i)) && (MemProperties.memoryTypes[i].propertyFlags & Properties) == Properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

	VkSampleCountFlagBits VK_Utils::Get_SampleCountFlagBits_FromInt(unsigned num)
	{
        if (num >= 0&&num<2)
        {
            return VK_SAMPLE_COUNT_1_BIT;
        }
        else if ( num >= 2&&num<4)
        {
            return VK_SAMPLE_COUNT_2_BIT;
        }
        else if (num >=4 && num < 8)
        {
            return VK_SAMPLE_COUNT_8_BIT;
        }
        else if (num >= 8 && num < 16)
        {
            return VK_SAMPLE_COUNT_16_BIT;
        }
        else if (num >= 16 && num < 32)
        {
            return VK_SAMPLE_COUNT_32_BIT;
        }
        else if (num >= 16 && num < 32)
        {
           return VK_SAMPLE_COUNT_64_BIT;
        }
        else
        {
            return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
        }

	}

	VkImageLayout VK_Utils::Translate_Texture_usage_type_To_Vulkan(const ENUM_TEXTURE_USAGE_TYPE& usage_type)
	{
        switch (usage_type)
        {
        case  ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_COLOR_ATTACHMENT:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            break;
        case ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_PRESENT_SWAPCHAIN:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            break;
        default:
            return VK_IMAGE_LAYOUT_UNDEFINED;
            break;
        }
	}

    VkImageType VK_Utils::Translate_Texture_type_To_Vulkan(const ENUM_TEXTURE_TYPE& type)
	{
        switch (type)
        {
		case ENUM_TEXTURE_TYPE::ENUM_TYPE_2D:
		case ENUM_TEXTURE_TYPE::ENUM_TYPE_2D_MULTISAMPLE:
		case ENUM_TEXTURE_TYPE::ENUM_TYPE_2D_DEPTH:
		{
            return VK_IMAGE_TYPE_2D;
			break;
		}
        default:
            return VK_IMAGE_TYPE_MAX_ENUM;
            break;
        }
	}

	VkFormat VK_Utils::Translate_API_DataTypeEnum_To_Vulkan(ENUM_RENDER_DATA_TYPE data_type)
	{
		switch (data_type)
		{
		case MXRender::ENUM_RENDER_DATA_TYPE::Float:
            return VK_FORMAT_R32_SFLOAT;
            break;
		case MXRender::ENUM_RENDER_DATA_TYPE::Half:
			return VK_FORMAT_R16_SFLOAT;
			break;
		case MXRender::ENUM_RENDER_DATA_TYPE::Mat3:
		case MXRender::ENUM_RENDER_DATA_TYPE::Mat4:
			return VK_FORMAT_R32_SFLOAT;
			break;
		case MXRender::ENUM_RENDER_DATA_TYPE::Int:
			return VK_FORMAT_R32_SINT;
			break;
		case MXRender::ENUM_RENDER_DATA_TYPE::Uint8:
            return VK_FORMAT_R8_UINT;
		case MXRender::ENUM_RENDER_DATA_TYPE::Uint10:
			return VK_FORMAT_R16_UINT;
			break;
		case MXRender::ENUM_RENDER_DATA_TYPE::Int16:
			return VK_FORMAT_R16_SINT;
			break;
		default:
			return VK_FORMAT_UNDEFINED;
			break;
		}
		return VK_FORMAT_UNDEFINED;
	}

}