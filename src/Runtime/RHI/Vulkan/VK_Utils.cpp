

#include "VK_Utils.h"
namespace MXRender
{


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

}