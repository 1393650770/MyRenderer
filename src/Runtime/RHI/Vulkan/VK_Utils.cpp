

#include "VK_Utils.h"
#include "VK_GraphicsContext.h"
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
	constexpr uint32_t fnv1a_32(char const* s, std::size_t count)
	{
		return ((count ? fnv1a_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
	}

	VkPipelineShaderStageCreateInfo VK_Utils::Pipeline_Shader_Stage_Create_Info(VkShaderStageFlagBits stage, VkShaderModule shaderModule)
	{
		VkPipelineShaderStageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		info.pNext = nullptr;

		//shader stage
		info.stage = stage;
		//module containing the code for this shader stage
		info.module = shaderModule;
		//the entry point of the shader
		info.pName = "main";
		return info;
	}

	VkPipelineVertexInputStateCreateInfo VK_Utils::Vertex_Input_State_Create_Info()
	{
		VkPipelineVertexInputStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		info.pNext = nullptr;

		//no vertex bindings or attributes
		info.vertexBindingDescriptionCount = 0;
		info.vertexAttributeDescriptionCount = 0;
		return info;
	}

	VkPipelineLayoutCreateInfo VK_Utils::Pipeline_Layout_Create_Info()
	{
		VkPipelineLayoutCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.pNext = nullptr;

		//empty defaults
		info.flags = 0;
		info.setLayoutCount = 0;
		info.pSetLayouts = nullptr;
		info.pushConstantRangeCount = 0;
		info.pPushConstantRanges = nullptr;
		return info;
	}

	uint32_t VK_Utils::Hash_Descriptor_Layout_Info(VkDescriptorSetLayoutCreateInfo* info)
	{
		std::stringstream ss;

		ss << info->flags;
		ss << info->bindingCount;

		for (auto i = 0u; i < info->bindingCount; i++) {
			const VkDescriptorSetLayoutBinding& binding = info->pBindings[i];

			ss << binding.binding;
			ss << binding.descriptorCount;
			ss << binding.descriptorType;
			ss << binding.stageFlags;
		}

		auto str = ss.str();

		return fnv1a_32(str.c_str(), str.length());
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

	void VK_Utils::Copy_VKBuffer(std::weak_ptr< VK_GraphicsContext> context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		context.lock()->copy_buffer(srcBuffer,dstBuffer, size);
	}



	void VK_Utils::Copy_VKBuffer(VK_GraphicsContext* context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		context->copy_buffer(srcBuffer, dstBuffer, size);
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

	VkSampler VK_Utils::Create_Linear_Sampler(VkPhysicalDevice physical_device, VkDevice device)
	{
		VkSampler sampler;
		VkPhysicalDeviceProperties physical_device_properties{};
		vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);

		VkSamplerCreateInfo samplerInfo{};

		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = physical_device_properties.limits.maxSamplerAnisotropy; // close :1.0f
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 8.0f; // todo: m_irradiance_texture_miplevels
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
		{
			throw std::runtime_error("vk create sampler");
		}
		return sampler;
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

	VkShaderStageFlagBits VK_Utils::Translate_API_ShaderTypeEnum_To_Vulkan(ENUM_SHADER_STAGE shader_type)
	{
		switch (shader_type)
		{
		case MXRender::Shader_Vertex:
			return VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case MXRender::Shader_Pixel:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		case MXRender::Shader_Geometry:
			return VK_SHADER_STAGE_GEOMETRY_BIT;
			break;
		case MXRender::Shader_Hull:
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			break;
		case MXRender::Shader_Domain:
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			break;
		case MXRender::NumStages:
			return VK_SHADER_STAGE_ALL_GRAPHICS;
			break;
		case MXRender::MaxNumSets:
			return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
			break;
		case MXRender::Shader_Compute:
			return VK_SHADER_STAGE_COMPUTE_BIT;
			break;
		case MXRender::Invalid:
			return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
			break;
		default:
			return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
			break;
		}
	}

	void VK_Utils::Transition_ImageLayout(std::weak_ptr< VK_GraphicsContext> context, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t layer_count, uint32_t miplevels, VkImageAspectFlags aspect_mask_bits)
	{
		if (context.expired())
		{
			return ;
		}

		VkCommandBuffer commandBuffer = context.lock()->begin_single_time_commands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = old_layout;
		barrier.newLayout = new_layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = aspect_mask_bits;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = miplevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layer_count;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		// for getGuidAndDepthOfMouseClickOnRenderSceneForUI() get depthimage
		else if (old_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL &&
			new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
			new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		// for generating mipmapped image
		else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else
		{
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		context.lock()->end_single_time_commands(commandBuffer);
	}

	void VK_Utils::Copy_Buffer_To_Image(std::weak_ptr< VK_GraphicsContext> context, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer_count)
	{
		if (context.expired())
		{
			return;
		}

		VkCommandBuffer commandBuffer = context.lock()->begin_single_time_commands();
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layer_count;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		context.lock()->end_single_time_commands(commandBuffer);
	}



}