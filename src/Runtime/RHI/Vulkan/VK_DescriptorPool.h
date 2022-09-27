#pragma once
#ifndef _VK_SHADER_
#define _VK_SHADER_
#include<vulkan/vulkan.h>
#include <string>
#include<vector>
#include<memory>
#include"glm/glm.hpp"
#include "../RenderEnum.h"
#include "../Shader.h"

namespace MXRender
{
    class VK_Device;

    class VK_DescriptorPool
    {
    private:
        ;
    protected:
        std::weak_ptr<VK_Device> Device;
        const unsigned int MaxDescriptorSets;
        VkDescriptorPool DescriptorPool;
    public:
        
        VK_DescriptorPool(std::shared_ptr<VK_Device> InDevice, unsigned int InMaxDescriptorSets);
        
        virtual ~VK_DescriptorPool();

        std::weak_ptr<VK_Device> GetDevice() const;
        unsigned int GetMaxDescriptorSets() const;
    };


}
#endif //_VK_SHADER_
