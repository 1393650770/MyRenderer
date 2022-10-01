#pragma once
#ifndef _VK_UTILS_
#define _VK_UTILS_

#include<vulkan/vulkan.h>
#include"glm/glm.hpp"
#include"VK_Device.h"
#include"../RenderEnum.h"
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>

namespace MXRender
{
    class VK_Utils
    {
    private:
       
    public:
        static void Create_VKBuffer(std::weak_ptr< VK_Device> Device, VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, VkBuffer& Buffer, VkDeviceMemory& BufferMemory);
        static uint32_t Find_MemoryType(std::weak_ptr< VK_Device> Device, uint32_t TypeFilter, VkMemoryPropertyFlags Properties);
        static VkSampleCountFlagBits Get_SampleCountFlagBits_FromInt(unsigned num);
        static VkImageLayout Translate_Texture_usage_type_To_Vulkan(const ENUM_TEXTURE_USAGE_TYPE& usage_type );
        static VkImageType Translate_Texture_type_To_Vulkan(const ENUM_TEXTURE_TYPE& type);
    };
    
}
#endif
