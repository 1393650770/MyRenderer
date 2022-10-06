#include "VK_Resource.h"
#include "VK_Device.h"
#include <stdexcept>

namespace MXRender
{



	void VK_TextureView::Create(std::shared_ptr<VK_Device> Device, VkImage InImage, VkImageViewType ViewType, VkImageAspectFlags AspectFlags, ENUM_TEXTURE_FORMAT MyFormat, VkFormat Format, int FirstMip, int NumMips, int ArraySliceIndex, int NumArraySlices, bool bUseIdentitySwizzle /*= false*/)
	{
		View = StaticCreate(Device, InImage, ViewType, AspectFlags, MyFormat, Format, FirstMip, NumMips, ArraySliceIndex, NumArraySlices, bUseIdentitySwizzle);
		Image = InImage;

	}



	void VK_TextureView::Destroy(std::shared_ptr<VK_Device> Device)
	{
		if (View)
		{
			vkDestroyImageView(Device->device, View, nullptr);
			Image = VK_NULL_HANDLE;
			View = VK_NULL_HANDLE;
			ViewId = 0;
		}
	}

	VkImageView VK_TextureView::StaticCreate(std::shared_ptr<VK_Device> Device, VkImage InImage, VkImageViewType ViewType, VkImageAspectFlags AspectFlags, ENUM_TEXTURE_FORMAT MyFormat, VkFormat Format, int FirstMip, int NumMips, int ArraySliceIndex, int NumArraySlices, bool bUseIdentitySwizzle)
	{
		VkImageView OutView = VK_NULL_HANDLE;
		VkImageViewCreateInfo ViewInfo;
		ViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ViewInfo.image = InImage;
		ViewInfo.viewType = ViewType;
		ViewInfo.format = Format;
		ViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ViewInfo.subresourceRange.baseMipLevel = FirstMip;
		ViewInfo.subresourceRange.levelCount = NumMips;
		ViewInfo.subresourceRange.baseArrayLayer = ArraySliceIndex;
		ViewInfo.subresourceRange.layerCount = NumArraySlices;
		ViewInfo.pNext=nullptr;
		if (vkCreateImageView(Device->device, &ViewInfo, nullptr, &OutView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
		return OutView;
	}

}
