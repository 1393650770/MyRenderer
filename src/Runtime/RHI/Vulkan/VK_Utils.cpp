

#include "VK_Utils.h"
namespace MXRender
{


    void VK_Utils::CreateVKBuffer(std::weak_ptr<VK_Device> Device, VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, VkBuffer& Buffer, VkDeviceMemory& BufferMemory)
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
        allocInfo.memoryTypeIndex = FindMemoryType(Device,memRequirements.memoryTypeBits, Properties);

        if (vkAllocateMemory(DeviceSharedPtr->device, &allocInfo, nullptr, &BufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(DeviceSharedPtr->device, Buffer, BufferMemory, 0);
    }

    uint32_t VK_Utils::FindMemoryType(std::weak_ptr<VK_Device> Device, uint32_t TypeFilter, VkMemoryPropertyFlags Properties)
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

}