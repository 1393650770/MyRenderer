#pragma once
#ifndef _VK_DESCRIPTORSETS_
#define _VK_DESCRIPTORSETS_
#include<vulkan/vulkan.h>
#include <string>
#include<vector>
#include<memory>
#include<unordered_map> 
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
        std::weak_ptr<VK_Device> device;
        const unsigned int max_descriptorsets;
        VkDescriptorPool descriptor_pool;
    public:
        
        VK_DescriptorPool(std::shared_ptr<VK_Device> InDevice, unsigned int InMaxDescriptorSets);
        
        virtual ~VK_DescriptorPool();

        std::weak_ptr<VK_Device> get_device() const;
        unsigned int get_max_descriptorsets() const;
        const VkDescriptorPool& get_descriptor_pool();
        bool allocate_descriptorset(VkDescriptorSetLayout Layout, VkDescriptorSet& OutSet);
        bool allocate_descriptorsets(const VkDescriptorSetAllocateInfo& InDescriptorSetAllocateInfo, VkDescriptorSet& OutSets);
    };

    class VK_DescriptorSetLayout
    {
    private:
    protected:
        const unsigned int max_bindings;
        std::weak_ptr<VK_Device> device;
        VkDescriptorSetLayout descriptorset_layout;
        std::vector< VkDescriptorSetLayoutBinding > layout_binding_array;
    public:
        VK_DescriptorSetLayout(std::shared_ptr<VK_Device> InDevice, unsigned int InMaxBindings=10);

        virtual ~VK_DescriptorSetLayout();

        void add_bindingdescriptor(unsigned int DescriptorSetIndex, const VkDescriptorSetLayoutBinding& BindingDescriptor);
        bool compile();
        VkDescriptorSetLayout& get_descriptorset_layout();
        std::weak_ptr<VK_Device> get_device() const;
    };

    class VK_VulkanLayout
    {
    private:
    protected:
        std::weak_ptr<VK_Device> device;
        std::vector < VK_DescriptorSetLayout> descriptorset_layout_array;
        VkPipelineLayout PipelineLayout;
    public:
        VK_VulkanLayout( std::shared_ptr<VK_Device> InDevice,unsigned int InDescriptorSetLayoutNum =0);
        virtual ~VK_VulkanLayout();
        bool compile();
        std::vector<VkDescriptorSetLayout> get_descriptorset_layout_data();
        VK_DescriptorSetLayout& get_descriptorset_layout_by_index(unsigned int Index);
    };





    class VK_DescriptorSets
    {
    private:
    protected:
        std::weak_ptr<VK_Device> device;
        VkDescriptorSet descriptorset;
    public:
        VK_DescriptorSets(std::shared_ptr<VK_Device> InDevice);
        virtual ~VK_DescriptorSets();

        bool update_descriptorsets(const std::string& SetKey, const std::vector<VkDescriptorSetLayout>& SetsLayout,
            std::vector<VkWriteDescriptorSet>& DSWriters, VkDescriptorSet& OutSets);
 
    };



}
#endif //_VK_DESCRIPTORSETS_
