#include"VK_Texture.h"
#include "../../Core/ConstDefine.h"
#include "VK_Define.h"
#include "VK_Utils.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

void VK_TextureView::Create(VK_Device& device, VkImage in_image, VkImageViewType view_type, VkImageAspectFlags aspect_flags, ENUM_TEXTURE_FORMAT vk_format, VkFormat format, UInt32 first_mip, UInt32 num_mips, UInt32 array_slice_index, UInt32 num_array_slices, Bool use_identity_swizzle)
{
	image = in_image;

	VkImageViewCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.image = image;
	create_info.viewType = view_type;
	create_info.format = format;
	create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.subresourceRange.aspectMask = aspect_flags;
	create_info.subresourceRange.baseMipLevel = first_mip;
	create_info.subresourceRange.levelCount = num_mips;
	create_info.subresourceRange.baseArrayLayer = array_slice_index;
	create_info.subresourceRange.layerCount = num_array_slices;

	if (vkCreateImageView(device.GetDevice(), &create_info, nullptr, &view) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image views!");
	}



}

void VK_TextureView::Destroy(VK_Device& device)
{

}

//VK_Texture::VK_Texture(std::vector<std::string>& cubemap_texture)
//{
//    if (is_valid())
//        return;
//
//    std::weak_ptr<VK_Device> device=Singleton<DefaultSetting>::get_instance().context->device;
//    if(device.expired()) return ;
//        
//	int miplevels=1;
//	type= ENUM_TEXTURE_TYPE::ENUM_TYPE_CUBE_MAP;
//    std::vector<std::shared_ptr<TextureData>> cubemap_testure_data(6);
//    for (int i=0;i<cubemap_texture.size();i++)
//    {
//        cubemap_testure_data[i]=RenderUtils::Load_Texture(cubemap_texture[i],true);
//    }
//	VkDeviceSize texture_layer_byte_size;
//	VkDeviceSize cube_byte_size;
//	VkFormat     vulkan_image_format;
//
//    switch (cubemap_testure_data[0]->format)
//    {       
//	case ENUM_TEXTURE_FORMAT::RGBA8S:
//	{
//		texture_layer_byte_size = cubemap_testure_data[0]->width * cubemap_testure_data[0]->height * 4;
//		vulkan_image_format = VK_FORMAT_R8G8B8A8_SRGB;
//		break;
//	}
//	case ENUM_TEXTURE_FORMAT::RGBA8U:
//	{
//		texture_layer_byte_size = cubemap_testure_data[0]->width * cubemap_testure_data[0]->height * 4;
//		vulkan_image_format = VK_FORMAT_R8G8B8A8_SRGB;
//		break;
//	}
//	default:
//	{
//		texture_layer_byte_size = VkDeviceSize(-1);
//		throw std::runtime_error("invalid texture_layer_byte_size");
//		break;
//	}
//    }
//    cube_byte_size = texture_layer_byte_size * 6;
//	uint32_t texture_image_width= cubemap_testure_data[0]->width;
//	uint32_t texture_image_height= cubemap_testure_data[0]->height;
//	VkImageCreateInfo image_create_info{};
//	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//	image_create_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
//	image_create_info.imageType = VK_IMAGE_TYPE_2D;
//	image_create_info.extent.width = static_cast<uint32_t>(texture_image_width);
//	image_create_info.extent.height = static_cast<uint32_t>(texture_image_height);
//	image_create_info.extent.depth = 1;
//	image_create_info.mipLevels = miplevels;
//	image_create_info.arrayLayers = 6;
//	image_create_info.format = vulkan_image_format;
//	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
//	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//	image_create_info.usage =
//		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
//	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
//	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//	if (vkCreateImage(device.lock()->device, &image_create_info, nullptr, &texture_image) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create image!");
//	}
//
//	VkMemoryRequirements memRequirements;
//	vkGetImageMemoryRequirements(device.lock()->device, texture_image, &memRequirements);
//
//	VkMemoryAllocateInfo allocInfo{};
//	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//	allocInfo.allocationSize = memRequirements.size;
//	allocInfo.memoryTypeIndex = VK_Utils::Find_MemoryType(device,memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
//
//	if (vkAllocateMemory(device.lock()->device, &allocInfo, nullptr, &texture_image_memory) != VK_SUCCESS) {
//		throw std::runtime_error("failed to allocate image memory!");
//	}
//
//	vkBindImageMemory(device.lock()->device, texture_image, texture_image_memory, 0);
//
//	VkBuffer       inefficient_staging_buffer;
//	VkDeviceMemory inefficient_staging_buffer_memory;
//	VK_Utils::Create_VKBuffer(device, 
//        cube_byte_size, 
//        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
//		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//		inefficient_staging_buffer,
//		inefficient_staging_buffer_memory);
//
//	void* data;
//	vkMapMemory(device.lock()->device, inefficient_staging_buffer_memory, 0, cube_byte_size, 0, &data);
//	for (int i = 0; i < 6; i++)
//	{
//		memcpy((void*)(static_cast<char*>(data) + texture_layer_byte_size * i),
//			cubemap_testure_data[i]->pixels,
//			static_cast<size_t>(texture_layer_byte_size));
//	}
//	vkUnmapMemory(device.lock()->device, inefficient_staging_buffer_memory);
//	VK_Utils::Transition_ImageLayout(Singleton<DefaultSetting>::get_instance().context, texture_image,
//		VK_IMAGE_LAYOUT_UNDEFINED,
//		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//		6,
//		miplevels,
//		VK_IMAGE_ASPECT_COLOR_BIT);
//	VK_Utils::Copy_Buffer_To_Image(Singleton<DefaultSetting>::get_instance().context, inefficient_staging_buffer,
//		texture_image,
//		static_cast<uint32_t>(texture_image_width),
//		static_cast<uint32_t>(texture_image_height),
//		6);
//	VK_Utils::Transition_ImageLayout(Singleton<DefaultSetting>::get_instance().context, texture_image,
//		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
//		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//		6,
//		miplevels,
//		VK_IMAGE_ASPECT_COLOR_BIT);
//	vkDestroyBuffer(device.lock()->device, inefficient_staging_buffer, nullptr);
//	vkFreeMemory(device.lock()->device, inefficient_staging_buffer_memory, nullptr);
//
//	texture_image_view = VK_Utils::Create_ImageView(device.lock()->device, 
//		texture_image,
//		vulkan_image_format,
//		VK_IMAGE_ASPECT_COLOR_BIT,
//		VK_IMAGE_VIEW_TYPE_CUBE,
//		6,
//		miplevels);
//
//	texture_image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//
//	texture_sampler=VK_Utils::Create_Linear_Sampler(device.lock()->gpu,device.lock()->device);
//}




VK_Texture::VK_Texture(VK_Device& in_device, const TextureDesc& texture_desc):Texture(texture_desc),
	device(&in_device)
{
	VkImageCreateInfo image_create_info;
	GenerateImageCreateInfo(image_create_info, in_device, texture_desc);

	CHECK_WITH_LOG(vkCreateImage(in_device.GetDevice(), &image_create_info, VULKAN_CPU_ALLOCATOR, &texture_image) != VK_SUCCESS,
					"RHI Error: failed to CreateImage !")
	texture_image_layout = image_create_info.initialLayout;

	const ENUM_VulkanAllocationFlags alloc_flags = ENUM_VulkanAllocationFlags::HostCached | ENUM_VulkanAllocationFlags::AutoBind;

	in_device.GetMemoryManager()->AllocateImageMemory(allocation,texture_image, alloc_flags,0);

	allocation.BindImage(&in_device,texture_image);

	VkImageViewCreateInfo image_view_create_info;
	GenerateViewCreateInfo(image_view_create_info, in_device, texture_desc,texture_image);

	CHECK_WITH_LOG(vkCreateImageView(in_device.GetDevice(), &image_view_create_info, VULKAN_CPU_ALLOCATOR, &texture_image_view) != VK_SUCCESS,
		"RHI Error: failed to CreateImageView !")

	texture_image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	texture_sampler = VK_Utils::Create_Linear_Sampler(in_device.GetGpu(), in_device.GetDevice());
}

 //void VK_Texture::load_dds(ENUM_TEXTURE_TYPE _type, const std::string& texture_path)
 //{
 //	switch (_type)
 //	{
 //	case MXRender::ENUM_TEXTURE_TYPE::ENUM_TYPE_NOT_VALID:
 //		break;
 //	case MXRender::ENUM_TEXTURE_TYPE::ENUM_TYPE_2D:
 //	case MXRender::ENUM_TEXTURE_TYPE::ENUM_TYPE_2D_MULTISAMPLE:
 //		load_dds_2d(_type,texture_path);
 //		break;
 //	case MXRender::ENUM_TEXTURE_TYPE::ENUM_TYPE_2D_DEPTH:
 //		break;
 //	case MXRender::ENUM_TEXTURE_TYPE::ENUM_TYPE_CUBE_MAP:
 //		load_dds_cubemap(_type,texture_path);
 //		break;
 //	case MXRender::ENUM_TEXTURE_TYPE::ENUM_TYPE_2D_DYNAMIC:
 //		break;
 //	default:
 //		break;
 //	}
 //}
 //
 //void VK_Texture::load_dds_cubemap(ENUM_TEXTURE_TYPE _type, const std::string& texture_path)
 //{
 //	gli::texture_cube cubemap(gli::load(texture_path));
 //	if (cubemap.empty()) {
 //		return;
 //	}
 //	std::weak_ptr<VK_Device> device = Singleton<DefaultSetting>::get_instance().context->device;
 //	if (device.expired())
 //		return;
 //	// 设置图像创建信息
 //	VkImageCreateInfo imageCreateInfo = {};
 //	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
 //	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
 //	imageCreateInfo.format = trans_gli_format_to_vulkan(cubemap.format()); // 假设Cube Map是RGBA8格式
 //	imageCreateInfo.extent = { static_cast<uint32_t>(cubemap.extent().x), static_cast<uint32_t>(cubemap.extent().y), 1 };
 //	imageCreateInfo.mipLevels = static_cast<uint32_t>(cubemap.levels());
 //	imageCreateInfo.arrayLayers = cubemap.faces(); // Cube Map需要6个数组层
 //	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
 //	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
 //	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
 //	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
 //	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
 //	imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; // 设置为Cube Map兼容
 //
 //	// 创建Vulkan图像
 //	VkResult result = vkCreateImage(device.lock()->device, &imageCreateInfo, nullptr, &texture_image);
 //	if (result != VK_SUCCESS) {
 //		return;
 //	}
 //
 //	// 分配图像内存
 //	VkMemoryRequirements memReqs;
 //	vkGetImageMemoryRequirements(device.lock()->device, texture_image, &memReqs);
 //
 //	VkMemoryAllocateInfo memAllocInfo = {};
 //	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
 //	memAllocInfo.allocationSize = memReqs.size;
 //	memAllocInfo.memoryTypeIndex = VK_Utils::Find_MemoryType(device, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
 //
 //	result = vkAllocateMemory(device.lock()->device, &memAllocInfo, nullptr, &texture_image_memory);
 //	if (result != VK_SUCCESS) {
 //		vkDestroyImage(device.lock()->device, texture_image, nullptr);
 //		return;
 //	}
 //
 //	// 绑定图像内存
 //	vkBindImageMemory(device.lock()->device, texture_image, texture_image_memory, 0);
 //
 //
 //	VkBuffer       inefficient_staging_buffer;
 //	VkDeviceMemory inefficient_staging_buffer_memory;
 //	VK_Utils::Create_VKBuffer(device,
 //		cubemap.size(),
 //		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
 //		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
 //		inefficient_staging_buffer,
 //		inefficient_staging_buffer_memory);
 //
 //
 //	void* data;
 //	vkMapMemory(device.lock()->device, inefficient_staging_buffer_memory, 0, cubemap.size(), 0, &data);
 //	memcpy(data, cubemap.data(), cubemap.size());
 //	vkUnmapMemory(device.lock()->device, inefficient_staging_buffer_memory);
 //	VK_Utils::Transition_ImageLayout(Singleton<DefaultSetting>::get_instance().context, texture_image,
 //		VK_IMAGE_LAYOUT_UNDEFINED,
 //		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
 //		6,
 //		cubemap.levels(),
 //		VK_IMAGE_ASPECT_COLOR_BIT);
 //
 //	VkDeviceSize bufferOffset = 0;
 //	for (uint32_t layer = 0; layer < cubemap.faces(); ++layer)
 //	{
 //
 //		for (uint32_t level = 0; level < cubemap.levels(); ++level)
 //		{
 //			VK_Utils::Copy_Buffer_To_Image(Singleton<DefaultSetting>::get_instance().context, inefficient_staging_buffer,
 //				texture_image,
 //				static_cast<uint32_t>(cubemap[layer][level].extent().x),
 //				static_cast<uint32_t>(cubemap[layer][level].extent().y),
 //				1,
 //				level,
 //				layer,
 //				bufferOffset);
 //			bufferOffset += cubemap[layer][level].size();
 //		}
 //	}
 //	VK_Utils::Transition_ImageLayout(Singleton<DefaultSetting>::get_instance().context, texture_image,
 //		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
 //		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
 //		6,
 //		cubemap.levels(),
 //		VK_IMAGE_ASPECT_COLOR_BIT);
 //	vkDestroyBuffer(device.lock()->device, inefficient_staging_buffer, nullptr);
 //	vkFreeMemory(device.lock()->device, inefficient_staging_buffer_memory, nullptr);
 //
 //	texture_image_view = VK_Utils::Create_ImageView(device.lock()->device,
 //		texture_image,
 //		imageCreateInfo.format,
 //		VK_IMAGE_ASPECT_COLOR_BIT,
 //		VK_IMAGE_VIEW_TYPE_CUBE,
 //		6,
 //		cubemap.levels());
 //	texture_image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
 //
 //	texture_sampler = VK_Utils::Create_Linear_Sampler(device.lock()->gpu, device.lock()->device);
 //}
 //
 //void VK_Texture::load_dds_2d(ENUM_TEXTURE_TYPE _type, const std::string& texture_path)
 //{
 //	gli::texture2d texture_2d(gli::load(texture_path));
 //	if (texture_2d.empty()) {
 //		return;
 //	}
 //	std::weak_ptr<VK_Device> device = Singleton<DefaultSetting>::get_instance().context->device;
 //	if (device.expired())
 //		return;
 //
 //	int miplevels = texture_2d.levels();
 //
 //
 //
 //	VkFormat     vulkan_image_format;
 //	unsigned layer_cout = 1;
 //	vulkan_image_format=trans_gli_format_to_vulkan(texture_2d.format());
 //
 //	uint32_t texture_image_width = texture_2d.extent().x;
 //	uint32_t texture_image_height = texture_2d.extent().y;
 //	VkImageCreateInfo image_create_info{};
 //	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
 //
 //	image_create_info.imageType = VK_IMAGE_TYPE_2D;
 //	image_create_info.extent.width = static_cast<uint32_t>(texture_image_width);
 //	image_create_info.extent.height = static_cast<uint32_t>(texture_image_height);
 //	image_create_info.extent.depth = 1;
 //	image_create_info.mipLevels = miplevels;
 //	image_create_info.arrayLayers = layer_cout;
 //	image_create_info.format = vulkan_image_format;
 //	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
 //	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
 //	image_create_info.usage =
 //		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
 //	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
 //	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
 //
 //	if (vkCreateImage(device.lock()->device, &image_create_info, nullptr, &texture_image) != VK_SUCCESS) {
 //		throw std::runtime_error("failed to create image!");
 //	}
 //
 //	VkMemoryRequirements memRequirements;
 //	vkGetImageMemoryRequirements(device.lock()->device, texture_image, &memRequirements);
 //
 //	VkMemoryAllocateInfo allocInfo{};
 //	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
 //	allocInfo.allocationSize = memRequirements.size;
 //	allocInfo.memoryTypeIndex = VK_Utils::Find_MemoryType(device, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
 //
 //	if (vkAllocateMemory(device.lock()->device, &allocInfo, nullptr, &texture_image_memory) != VK_SUCCESS) {
 //		throw std::runtime_error("failed to allocate image memory!");
 //	}
 //
 //	vkBindImageMemory(device.lock()->device, texture_image, texture_image_memory, 0);
 //
 //	VkBuffer       inefficient_staging_buffer;
 //	VkDeviceMemory inefficient_staging_buffer_memory;
 //	VK_Utils::Create_VKBuffer(device,
 //		texture_2d.size(),
 //		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
 //		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
 //		inefficient_staging_buffer,
 //		inefficient_staging_buffer_memory);
 //
 //	void* data;
 //	vkMapMemory(device.lock()->device, inefficient_staging_buffer_memory, 0, texture_2d.size(), 0, &data);
 //
 //	memcpy((void*)(static_cast<char*>(data)),
 //		texture_2d.data(),
 //		static_cast<size_t>(texture_2d.size()));
 //		
 //	vkUnmapMemory(device.lock()->device, inefficient_staging_buffer_memory);
 //	VK_Utils::Transition_ImageLayout(Singleton<DefaultSetting>::get_instance().context, texture_image,
 //		VK_IMAGE_LAYOUT_UNDEFINED,
 //		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
 //		layer_cout,
 //		miplevels,
 //		VK_IMAGE_ASPECT_COLOR_BIT);
 //
 //	VkDeviceSize bufferOffset = 0;
 //	for (uint32_t level = 0; level < texture_2d.levels(); ++level)
 //	{
 //		VK_Utils::Copy_Buffer_To_Image(Singleton<DefaultSetting>::get_instance().context, inefficient_staging_buffer,
 //			texture_image,
 //			static_cast<uint32_t>(texture_2d[level].extent().x),
 //			static_cast<uint32_t>(texture_2d[level].extent().y),
 //			1,
 //			level,
 //			0,
 //			bufferOffset);
 //		bufferOffset += texture_2d[level].size();
 //	}
 //		
 //	VK_Utils::Transition_ImageLayout(Singleton<DefaultSetting>::get_instance().context, texture_image,
 //		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
 //		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
 //		layer_cout,
 //		miplevels,
 //		VK_IMAGE_ASPECT_COLOR_BIT);
 //	vkDestroyBuffer(device.lock()->device, inefficient_staging_buffer, nullptr);
 //	vkFreeMemory(device.lock()->device, inefficient_staging_buffer_memory, nullptr);
 //
 //	texture_image_view = VK_Utils::Create_ImageView(device.lock()->device,
 //		texture_image,
 //		vulkan_image_format,
 //		VK_IMAGE_ASPECT_COLOR_BIT,
 //		VK_IMAGE_VIEW_TYPE_2D,
 //		layer_cout,
 //		miplevels);
 //	texture_image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
 //
 //	texture_sampler = VK_Utils::Create_Linear_Sampler(device.lock()->gpu, device.lock()->device);
 //}
 //
 //void VK_Texture::load_common_2d(const std::string& texture_path)
 //{
 //
 //
 //	std::weak_ptr<VK_Device> device = Singleton<DefaultSetting>::get_instance().context->device;
 //	if (device.expired()) return;
 //
 //	int miplevels = 1;
 //	std::vector<std::shared_ptr<TextureData>> texture_data(1);
 //
 //	texture_data[0] = RenderUtils::Load_Texture(texture_path, true);
 //
 //	VkDeviceSize texture_layer_byte_size;
 //	VkDeviceSize texture_byte_size;
 //	VkFormat     vulkan_image_format;
 //	unsigned layer_cout = 1;
 //	switch (texture_data[0]->format)
 //	{
 //	case ENUM_TEXTURE_FORMAT::RGBA8S:
 //	{
 //		texture_layer_byte_size = texture_data[0]->width * texture_data[0]->height * 4;
 //		vulkan_image_format = VK_FORMAT_R8G8B8A8_SRGB;
 //		break;
 //	}
 //	case ENUM_TEXTURE_FORMAT::RGBA8U:
 //	{
 //		texture_layer_byte_size = texture_data[0]->width * texture_data[0]->height * 4;
 //		vulkan_image_format = VK_FORMAT_R8G8B8A8_SRGB;
 //		break;
 //	}
 //	default:
 //	{
 //		texture_layer_byte_size = VkDeviceSize(-1);
 //		throw std::runtime_error("invalid texture_layer_byte_size");
 //		break;
 //	}
 //	}
 //	texture_byte_size = texture_layer_byte_size;
 //	uint32_t texture_image_width = texture_data[0]->width;
 //	uint32_t texture_image_height = texture_data[0]->height;
 //	VkImageCreateInfo image_create_info{};
 //	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
 //
 //	image_create_info.imageType = VK_IMAGE_TYPE_2D;
 //	image_create_info.extent.width = static_cast<uint32_t>(texture_image_width);
 //	image_create_info.extent.height = static_cast<uint32_t>(texture_image_height);
 //	image_create_info.extent.depth = 1;
 //	image_create_info.mipLevels = miplevels;
 //	image_create_info.arrayLayers = layer_cout;
 //	image_create_info.format = vulkan_image_format;
 //	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
 //	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
 //	image_create_info.usage =
 //		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
 //	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
 //	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
 //
 //	if (vkCreateImage(device.lock()->device, &image_create_info, nullptr, &texture_image) != VK_SUCCESS) {
 //		throw std::runtime_error("failed to create image!");
 //	}
 //
 //	VkMemoryRequirements memRequirements;
 //	vkGetImageMemoryRequirements(device.lock()->device, texture_image, &memRequirements);
 //
 //	VkMemoryAllocateInfo allocInfo{};
 //	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
 //	allocInfo.allocationSize = memRequirements.size;
 //	allocInfo.memoryTypeIndex = VK_Utils::Find_MemoryType(device, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
 //
 //	if (vkAllocateMemory(device.lock()->device, &allocInfo, nullptr, &texture_image_memory) != VK_SUCCESS) {
 //		throw std::runtime_error("failed to allocate image memory!");
 //	}
 //
 //	vkBindImageMemory(device.lock()->device, texture_image, texture_image_memory, 0);
 //
 //	VkBuffer       inefficient_staging_buffer;
 //	VkDeviceMemory inefficient_staging_buffer_memory;
 //	VK_Utils::Create_VKBuffer(device,
 //		texture_byte_size,
 //		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
 //		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
 //		inefficient_staging_buffer,
 //		inefficient_staging_buffer_memory);
 //
 //	void* data;
 //	vkMapMemory(device.lock()->device, inefficient_staging_buffer_memory, 0, texture_byte_size, 0, &data);
 //	for (int i = 0; i < layer_cout; i++)
 //	{
 //		memcpy((void*)(static_cast<char*>(data) + texture_layer_byte_size * i),
 //			texture_data[i]->pixels,
 //			static_cast<size_t>(texture_layer_byte_size));
 //	}
 //	vkUnmapMemory(device.lock()->device, inefficient_staging_buffer_memory);
 //	VK_Utils::Transition_ImageLayout(Singleton<DefaultSetting>::get_instance().context, texture_image,
 //		VK_IMAGE_LAYOUT_UNDEFINED,
 //		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
 //		layer_cout,
 //		miplevels,
 //		VK_IMAGE_ASPECT_COLOR_BIT);
 //	VK_Utils::Copy_Buffer_To_Image(Singleton<DefaultSetting>::get_instance().context, inefficient_staging_buffer,
 //		texture_image,
 //		static_cast<uint32_t>(texture_image_width),
 //		static_cast<uint32_t>(texture_image_height),
 //		layer_cout);
 //	VK_Utils::Transition_ImageLayout(Singleton<DefaultSetting>::get_instance().context, texture_image,
 //		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
 //		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
 //		layer_cout,
 //		miplevels,
 //		VK_IMAGE_ASPECT_COLOR_BIT);
 //	vkDestroyBuffer(device.lock()->device, inefficient_staging_buffer, nullptr);
 //	vkFreeMemory(device.lock()->device, inefficient_staging_buffer_memory, nullptr);
 //
 //	texture_image_view = VK_Utils::Create_ImageView(device.lock()->device,
 //		texture_image,
 //		vulkan_image_format,
 //		VK_IMAGE_ASPECT_COLOR_BIT,
 //		VK_IMAGE_VIEW_TYPE_2D,
 //		layer_cout,
 //		miplevels);
 //
 //	texture_sampler = VK_Utils::Create_Linear_Sampler(device.lock()->gpu, device.lock()->device);
 //	texture_image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
 //
 //}

VkFormat VK_Texture::trans_gli_format_to_vulkan(gli::format format)
{
	switch (format)
	{
	case gli::FORMAT_RGBA8_UNORM_PACK8:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case gli::FORMAT_RGB8_UNORM_PACK8:
		return VK_FORMAT_R8G8B8_UNORM;
	case gli::FORMAT_RGBA16_SFLOAT_PACK16:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case gli::FORMAT_RGBA32_SFLOAT_PACK32:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case gli::FORMAT_R8_UNORM_PACK8:
		return VK_FORMAT_R8_UNORM;
	case gli::FORMAT_R16_SFLOAT_PACK16:
		return VK_FORMAT_R16_SFLOAT;
	case gli::FORMAT_R32_SFLOAT_PACK32:
		return VK_FORMAT_R32_SFLOAT;
	case gli::FORMAT_RG8_UNORM_PACK8:
		return VK_FORMAT_R8G8_UNORM;
	case gli::FORMAT_RG16_SFLOAT_PACK16:
		return VK_FORMAT_R16G16_SFLOAT;
	case gli::FORMAT_RG32_SFLOAT_PACK32:
		return VK_FORMAT_R32G32_SFLOAT;
	case gli::FORMAT_RGB_DXT1_UNORM_BLOCK8:
		return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
	case gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8:
		return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
	case gli::FORMAT_RGBA_DXT3_UNORM_BLOCK16:
		return VK_FORMAT_BC2_UNORM_BLOCK;
	case gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16:
		return VK_FORMAT_BC3_UNORM_BLOCK;
	case gli::FORMAT_UNDEFINED:
		return VK_FORMAT_UNDEFINED;
		break;
	case gli::FORMAT_RG4_UNORM_PACK8:
		return VK_FORMAT_R4G4_UNORM_PACK8;
		break;
	case gli::FORMAT_RGBA4_UNORM_PACK16:
		break;
	case gli::FORMAT_BGRA4_UNORM_PACK16:
		break;
	case gli::FORMAT_R5G6B5_UNORM_PACK16:
		break;
	case gli::FORMAT_B5G6R5_UNORM_PACK16:
		break;
	case gli::FORMAT_RGB5A1_UNORM_PACK16:
		break;
	case gli::FORMAT_BGR5A1_UNORM_PACK16:
		break;
	case gli::FORMAT_A1RGB5_UNORM_PACK16:
		break;
	case gli::FORMAT_R8_SNORM_PACK8:
		break;
	case gli::FORMAT_R8_USCALED_PACK8:
		break;
	case gli::FORMAT_R8_SSCALED_PACK8:
		break;
	case gli::FORMAT_R8_UINT_PACK8:
		break;
	case gli::FORMAT_R8_SINT_PACK8:
		break;
	case gli::FORMAT_R8_SRGB_PACK8:
		break;
	case gli::FORMAT_RG8_SNORM_PACK8:
		break;
	case gli::FORMAT_RG8_USCALED_PACK8:
		break;
	case gli::FORMAT_RG8_SSCALED_PACK8:
		break;
	case gli::FORMAT_RG8_UINT_PACK8:
		break;
	case gli::FORMAT_RG8_SINT_PACK8:
		break;
	case gli::FORMAT_RG8_SRGB_PACK8:
		break;
	case gli::FORMAT_RGB8_SNORM_PACK8:
		break;
	case gli::FORMAT_RGB8_USCALED_PACK8:
		break;
	case gli::FORMAT_RGB8_SSCALED_PACK8:
		break;
	case gli::FORMAT_RGB8_UINT_PACK8:
		break;
	case gli::FORMAT_RGB8_SINT_PACK8:
		break;
	case gli::FORMAT_RGB8_SRGB_PACK8:
		break;
	case gli::FORMAT_BGR8_UNORM_PACK8:
		break;
	case gli::FORMAT_BGR8_SNORM_PACK8:
		break;
	case gli::FORMAT_BGR8_USCALED_PACK8:
		break;
	case gli::FORMAT_BGR8_SSCALED_PACK8:
		break;
	case gli::FORMAT_BGR8_UINT_PACK8:
		break;
	case gli::FORMAT_BGR8_SINT_PACK8:
		break;
	case gli::FORMAT_BGR8_SRGB_PACK8:
		break;
	case gli::FORMAT_RGBA8_SNORM_PACK8:
		break;
	case gli::FORMAT_RGBA8_USCALED_PACK8:
		break;
	case gli::FORMAT_RGBA8_SSCALED_PACK8:
		break;
	case gli::FORMAT_RGBA8_UINT_PACK8:
		break;
	case gli::FORMAT_RGBA8_SINT_PACK8:
		break;
	case gli::FORMAT_RGBA8_SRGB_PACK8:
		break;
	case gli::FORMAT_BGRA8_UNORM_PACK8:
		break;
	case gli::FORMAT_BGRA8_SNORM_PACK8:
		break;
	case gli::FORMAT_BGRA8_USCALED_PACK8:
		break;
	case gli::FORMAT_BGRA8_SSCALED_PACK8:
		break;
	case gli::FORMAT_BGRA8_UINT_PACK8:
		break;
	case gli::FORMAT_BGRA8_SINT_PACK8:
		break;
	case gli::FORMAT_BGRA8_SRGB_PACK8:
		break;
	case gli::FORMAT_RGBA8_UNORM_PACK32:
		break;
	case gli::FORMAT_RGBA8_SNORM_PACK32:
		break;
	case gli::FORMAT_RGBA8_USCALED_PACK32:
		break;
	case gli::FORMAT_RGBA8_SSCALED_PACK32:
		break;
	case gli::FORMAT_RGBA8_UINT_PACK32:
		break;
	case gli::FORMAT_RGBA8_SINT_PACK32:
		break;
	case gli::FORMAT_RGBA8_SRGB_PACK32:
		break;
	case gli::FORMAT_RGB10A2_UNORM_PACK32:
		break;
	case gli::FORMAT_RGB10A2_SNORM_PACK32:
		break;
	case gli::FORMAT_RGB10A2_USCALED_PACK32:
		break;
	case gli::FORMAT_RGB10A2_SSCALED_PACK32:
		break;
	case gli::FORMAT_RGB10A2_UINT_PACK32:
		break;
	case gli::FORMAT_RGB10A2_SINT_PACK32:
		break;
	case gli::FORMAT_BGR10A2_UNORM_PACK32:
		break;
	case gli::FORMAT_BGR10A2_SNORM_PACK32:
		break;
	case gli::FORMAT_BGR10A2_USCALED_PACK32:
		break;
	case gli::FORMAT_BGR10A2_SSCALED_PACK32:
		break;
	case gli::FORMAT_BGR10A2_UINT_PACK32:
		break;
	case gli::FORMAT_BGR10A2_SINT_PACK32:
		break;
	case gli::FORMAT_R16_UNORM_PACK16:
		break;
	case gli::FORMAT_R16_SNORM_PACK16:
		break;
	case gli::FORMAT_R16_USCALED_PACK16:
		break;
	case gli::FORMAT_R16_SSCALED_PACK16:
		break;
	case gli::FORMAT_R16_UINT_PACK16:
		break;
	case gli::FORMAT_R16_SINT_PACK16:
		break;
	case gli::FORMAT_RG16_UNORM_PACK16:
		break;
	case gli::FORMAT_RG16_SNORM_PACK16:
		break;
	case gli::FORMAT_RG16_USCALED_PACK16:
		break;
	case gli::FORMAT_RG16_SSCALED_PACK16:
		break;
	case gli::FORMAT_RG16_UINT_PACK16:
		break;
	case gli::FORMAT_RG16_SINT_PACK16:
		break;
	case gli::FORMAT_RGB16_UNORM_PACK16:
		break;
	case gli::FORMAT_RGB16_SNORM_PACK16:
		break;
	case gli::FORMAT_RGB16_USCALED_PACK16:
		break;
	case gli::FORMAT_RGB16_SSCALED_PACK16:
		break;
	case gli::FORMAT_RGB16_UINT_PACK16:
		break;
	case gli::FORMAT_RGB16_SINT_PACK16:
		break;
	case gli::FORMAT_RGB16_SFLOAT_PACK16:
		break;
	case gli::FORMAT_RGBA16_UNORM_PACK16:
		break;
	case gli::FORMAT_RGBA16_SNORM_PACK16:
		break;
	case gli::FORMAT_RGBA16_USCALED_PACK16:
		break;
	case gli::FORMAT_RGBA16_SSCALED_PACK16:
		break;
	case gli::FORMAT_RGBA16_UINT_PACK16:
		break;
	case gli::FORMAT_RGBA16_SINT_PACK16:
		break;
	case gli::FORMAT_R32_UINT_PACK32:
		break;
	case gli::FORMAT_R32_SINT_PACK32:
		break;
	case gli::FORMAT_RG32_UINT_PACK32:
		break;
	case gli::FORMAT_RG32_SINT_PACK32:
		break;
	case gli::FORMAT_RGB32_UINT_PACK32:
		break;
	case gli::FORMAT_RGB32_SINT_PACK32:
		break;
	case gli::FORMAT_RGB32_SFLOAT_PACK32:
		break;
	case gli::FORMAT_RGBA32_UINT_PACK32:
		break;
	case gli::FORMAT_RGBA32_SINT_PACK32:
		break;
	case gli::FORMAT_R64_UINT_PACK64:
		break;
	case gli::FORMAT_R64_SINT_PACK64:
		break;
	case gli::FORMAT_R64_SFLOAT_PACK64:
		break;
	case gli::FORMAT_RG64_UINT_PACK64:
		break;
	case gli::FORMAT_RG64_SINT_PACK64:
		break;
	case gli::FORMAT_RG64_SFLOAT_PACK64:
		break;
	case gli::FORMAT_RGB64_UINT_PACK64:
		break;
	case gli::FORMAT_RGB64_SINT_PACK64:
		break;
	case gli::FORMAT_RGB64_SFLOAT_PACK64:
		break;
	case gli::FORMAT_RGBA64_UINT_PACK64:
		break;
	case gli::FORMAT_RGBA64_SINT_PACK64:
		break;
	case gli::FORMAT_RGBA64_SFLOAT_PACK64:
		break;
	case gli::FORMAT_RG11B10_UFLOAT_PACK32:
		break;
	case gli::FORMAT_RGB9E5_UFLOAT_PACK32:
		break;
	case gli::FORMAT_D16_UNORM_PACK16:
		break;
	case gli::FORMAT_D24_UNORM_PACK32:
		break;
	case gli::FORMAT_D32_SFLOAT_PACK32:
		break;
	case gli::FORMAT_S8_UINT_PACK8:
		break;
	case gli::FORMAT_D16_UNORM_S8_UINT_PACK32:
		break;
	case gli::FORMAT_D24_UNORM_S8_UINT_PACK32:
		break;
	case gli::FORMAT_D32_SFLOAT_S8_UINT_PACK64:
		break;
	case gli::FORMAT_RGB_DXT1_SRGB_BLOCK8:
		break;
	case gli::FORMAT_RGBA_DXT1_SRGB_BLOCK8:
		break;
	case gli::FORMAT_RGBA_DXT3_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_DXT5_SRGB_BLOCK16:
		break;
	case gli::FORMAT_R_ATI1N_UNORM_BLOCK8:
		break;
	case gli::FORMAT_R_ATI1N_SNORM_BLOCK8:
		break;
	case gli::FORMAT_RG_ATI2N_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RG_ATI2N_SNORM_BLOCK16:
		break;
	case gli::FORMAT_RGB_BP_UFLOAT_BLOCK16:
		break;
	case gli::FORMAT_RGB_BP_SFLOAT_BLOCK16:
		break;
	case gli::FORMAT_RGBA_BP_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_BP_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGB_ETC2_UNORM_BLOCK8:
		break;
	case gli::FORMAT_RGB_ETC2_SRGB_BLOCK8:
		break;
	case gli::FORMAT_RGBA_ETC2_UNORM_BLOCK8:
		break;
	case gli::FORMAT_RGBA_ETC2_SRGB_BLOCK8:
		break;
	case gli::FORMAT_RGBA_ETC2_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ETC2_SRGB_BLOCK16:
		break;
	case gli::FORMAT_R_EAC_UNORM_BLOCK8:
		break;
	case gli::FORMAT_R_EAC_SNORM_BLOCK8:
		break;
	case gli::FORMAT_RG_EAC_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RG_EAC_SNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_4X4_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_4X4_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_5X4_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_5X4_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_5X5_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_5X5_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_6X5_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_6X5_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_6X6_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_6X6_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_8X5_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_8X5_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_8X6_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_8X6_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_8X8_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_8X8_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_10X5_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_10X5_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_10X6_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_10X6_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_10X8_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_10X8_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_10X10_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_10X10_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_12X10_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_12X10_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_12X12_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_12X12_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGB_PVRTC1_8X8_UNORM_BLOCK32:
		break;
	case gli::FORMAT_RGB_PVRTC1_8X8_SRGB_BLOCK32:
		break;
	case gli::FORMAT_RGB_PVRTC1_16X8_UNORM_BLOCK32:
		break;
	case gli::FORMAT_RGB_PVRTC1_16X8_SRGB_BLOCK32:
		break;
	case gli::FORMAT_RGBA_PVRTC1_8X8_UNORM_BLOCK32:
		break;
	case gli::FORMAT_RGBA_PVRTC1_8X8_SRGB_BLOCK32:
		break;
	case gli::FORMAT_RGBA_PVRTC1_16X8_UNORM_BLOCK32:
		break;
	case gli::FORMAT_RGBA_PVRTC1_16X8_SRGB_BLOCK32:
		break;
	case gli::FORMAT_RGBA_PVRTC2_4X4_UNORM_BLOCK8:
		break;
	case gli::FORMAT_RGBA_PVRTC2_4X4_SRGB_BLOCK8:
		break;
	case gli::FORMAT_RGBA_PVRTC2_8X4_UNORM_BLOCK8:
		break;
	case gli::FORMAT_RGBA_PVRTC2_8X4_SRGB_BLOCK8:
		break;
	case gli::FORMAT_RGB_ETC_UNORM_BLOCK8:
		break;
	case gli::FORMAT_RGB_ATC_UNORM_BLOCK8:
		break;
	case gli::FORMAT_RGBA_ATCA_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ATCI_UNORM_BLOCK16:
		break;
	case gli::FORMAT_L8_UNORM_PACK8:
		break;
	case gli::FORMAT_A8_UNORM_PACK8:
		break;
	case gli::FORMAT_LA8_UNORM_PACK8:
		break;
	case gli::FORMAT_L16_UNORM_PACK16:
		break;
	case gli::FORMAT_A16_UNORM_PACK16:
		break;
	case gli::FORMAT_LA16_UNORM_PACK16:
		break;
	case gli::FORMAT_BGR8_UNORM_PACK32:
		break;
	case gli::FORMAT_BGR8_SRGB_PACK32:
		break;
	case gli::FORMAT_RG3B2_UNORM_PACK8:
		break;
	default:
		break;
	}
	std::abort();
	return VK_FORMAT_UNDEFINED;
}

std::string VK_Texture::get_file_extension(const std::string& filename)
{
	size_t pos = filename.rfind('.');
	if (pos != std::string::npos) 
	{
		return filename.substr(pos + 1);
	}
	else 
	{
		return "";
	}
}

VK_Texture::~VK_Texture()
{

}

void VK_Texture::GenerateImageCreateInfo(VkImageCreateInfo& image_create_info, VK_Device& in_device, const TextureDesc& desc)
{
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

	VkImageType image_type= VK_Utils::Translate_Texture_type_To_Vulkan(desc.type);
	VkFormat format = VK_Utils::Translate_Texture_Format_To_Vulkan(desc.format);
	VkImageCreateFlags flag = VK_Utils::Translate_Texture_type_To_VulkanCreateFlags(desc.type);
	VkSampleCountFlagBits sample_count = VK_Utils::Translate_Texture_SampleCount_To_Vulkan(desc.samples);
	VkImageUsageFlagBits usage = VK_Utils::Translate_Texture_usage_type_To_VulkanUsageFlagsBits(desc.usage);
	image_create_info.imageType = image_type;
	image_create_info.extent.width = static_cast<uint32_t>(desc.width);
	image_create_info.extent.height = static_cast<uint32_t>(desc.height);
	image_create_info.extent.depth = static_cast<uint32_t>(desc.depth);
	image_create_info.mipLevels = static_cast<uint32_t>(desc.mip_level);
	image_create_info.arrayLayers = static_cast<uint32_t>(desc.layer_count);
	image_create_info.format = format;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_create_info.usage =
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | usage;
	image_create_info.samples = sample_count;
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_create_info.flags = flag;
}

void VK_Texture::GenerateViewCreateInfo(VkImageViewCreateInfo& view_create_info, VK_Device& in_device, const TextureDesc& desc,VkImage& texture_image)
{
	view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_create_info.image = texture_image;
	view_create_info.viewType = VK_Utils::Translate_Texture_type_To_VulkanImageViewType(desc.type);
	view_create_info.format = VK_Utils::Translate_Texture_Format_To_Vulkan(desc.format);
	view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
	view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
	view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
	view_create_info.components.a = VK_COMPONENT_SWIZZLE_A;
	view_create_info.subresourceRange.aspectMask = VK_Utils::Translate_Texture_type_To_VulkanImageAspectFlags(desc.type);
	view_create_info.subresourceRange.baseMipLevel = 0;
	view_create_info.subresourceRange.levelCount = static_cast<uint32_t>(desc.mip_level);
	view_create_info.subresourceRange.baseArrayLayer = 0;
	view_create_info.subresourceRange.layerCount = static_cast<uint32_t>(desc.layer_count);

}


void VK_Texture::UpdateTextureData(CONST TextureDataPayload& texture_data_payload)
{
	if (texture_data_payload.data == nullptr|| texture_data_payload.data_size == 0)
	{
		return;
	}

	//	VkBuffer       inefficient_staging_buffer;
	//	VkDeviceMemory inefficient_staging_buffer_memory;
	//	VK_Utils::Create_VKBuffer(device,
	//		cubemap.size(),
	//		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	//		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	//		inefficient_staging_buffer,
	//		inefficient_staging_buffer_memory);
	//
	//
	//	void* data;
	//	vkMapMemory(device.lock()->device, inefficient_staging_buffer_memory, 0, cubemap.size(), 0, &data);
	//	memcpy(data, cubemap.data(), cubemap.size());
	//	vkUnmapMemory(device.lock()->device, inefficient_staging_buffer_memory);
	//	VK_Utils::Transition_ImageLayout(Singleton<DefaultSetting>::get_instance().context, texture_image,
	//		VK_IMAGE_LAYOUT_UNDEFINED,
	//		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//		6,
	//		cubemap.levels(),
	//		VK_IMAGE_ASPECT_COLOR_BIT);
	//
	//	VkDeviceSize bufferOffset = 0;
	//	for (uint32_t layer = 0; layer < cubemap.faces(); ++layer)
	//	{
	//
	//		for (uint32_t level = 0; level < cubemap.levels(); ++level)
	//		{
	//			VK_Utils::Copy_Buffer_To_Image(Singleton<DefaultSetting>::get_instance().context, inefficient_staging_buffer,
	//				texture_image,
	//				static_cast<uint32_t>(cubemap[layer][level].extent().x),
	//				static_cast<uint32_t>(cubemap[layer][level].extent().y),
	//				1,
	//				level,
	//				layer,
	//				bufferOffset);
	//			bufferOffset += cubemap[layer][level].size();
	//		}
	//	}
	//	VK_Utils::Transition_ImageLayout(Singleton<DefaultSetting>::get_instance().context, texture_image,
	//		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	//		6,
	//		cubemap.levels(),
	//		VK_IMAGE_ASPECT_COLOR_BIT);
	//	vkDestroyBuffer(device.lock()->device, inefficient_staging_buffer, nullptr);
	//	vkFreeMemory(device.lock()->device, inefficient_staging_buffer_memory, nullptr);
	//
	//	texture_image_view = VK_Utils::Create_ImageView(device.lock()->device,
	//		texture_image,
	//		imageCreateInfo.format,
	//		VK_IMAGE_ASPECT_COLOR_BIT,
	//		VK_IMAGE_VIEW_TYPE_CUBE,
	//		6,
	//		cubemap.levels());
	//	texture_image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	//
	//	texture_sampler = VK_Utils::Create_Linear_Sampler(device.lock()->gpu, device.lock()->device);

}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
