#pragma once
#ifndef _VK_DESCRIPTORSETS_
#define _VK_DESCRIPTORSETS_
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
    class VK_DescriptorPool;
    class VK_DescriptorSetLayout;


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

        std::weak_ptr<VK_Device> getDevice() const;
        unsigned int getMaxDescriptorSets() const;
        const VkDescriptorPool& getDescriptorPool();
        bool allocateDescriptorSet(VkDescriptorSetLayout Layout, VkDescriptorSet& OutSet);
    };

    class VK_DescriptorSetLayout
    {
    private:
    protected:
        const unsigned int MaxBindings;
        std::weak_ptr<VK_Device> Device;
        VkDescriptorSetLayout DescriptorSetLayout;
        std::vector< VkDescriptorSetLayoutBinding > UboLayoutBindingArray;
    public:
        VK_DescriptorSetLayout(std::shared_ptr<VK_Device> InDevice, unsigned int InMaxBindings=10);

        virtual ~VK_DescriptorSetLayout();

        void addBindingDescriptor(unsigned int DescriptorSetIndex, const VkDescriptorSetLayoutBinding& BindingDescriptor);
        bool compile();
        VkDescriptorSetLayout& getDescriptorSetLayout();
        std::weak_ptr<VK_Device> getDevice() const;
    };

    class VK_VulkanLayout
    {
    private:
    protected:
        std::weak_ptr<VK_Device> Device;
        std::vector<VK_DescriptorSetLayout> DescriptorSetLayoutArray;
        VkPipelineLayout PipelineLayout;
    public:
        VK_VulkanLayout(std::shared_ptr<VK_Device> InDevice,unsigned int InDescriptorSetLayoutNum=0);
        virtual ~VK_VulkanLayout();
        bool compile();
        std::vector<VkDescriptorSetLayout&> getDescriptorSetLayoutData();
        VK_DescriptorSetLayout& getDescriptorSetLayoutByIndex(unsigned int Index);
    };



}
#endif //_VK_DESCRIPTORSETS_
