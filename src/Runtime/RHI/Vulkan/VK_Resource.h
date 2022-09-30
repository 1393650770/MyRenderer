#pragma once

#ifndef _VK_RESOURCE_
#define _VK_RESOURCE_
#include <vulkan/vulkan.h>
#include "../RenderEnum.h"
#include <memory>

namespace MXRender
{
	class VK_Device;
	struct VK_TextureView
	{
		VK_TextureView()
			: View(VK_NULL_HANDLE)
			, Image(VK_NULL_HANDLE)
			, ViewId(0)
		{
		}

		void Create(std::shared_ptr<VK_Device> Device, VkImage InImage, VkImageViewType ViewType, VkImageAspectFlags AspectFlags, ENUM_TEXTURE_FORMAT MyFormat, VkFormat Format, int FirstMip, int NumMips, int ArraySliceIndex, int NumArraySlices, bool bUseIdentitySwizzle = false);
		void Destroy(std::shared_ptr<VK_Device> Device);

		VkImageView View;
		VkImage Image;
		int ViewId;

	private:
		static VkImageView StaticCreate(std::shared_ptr<VK_Device> Device, VkImage InImage, VkImageViewType ViewType, VkImageAspectFlags AspectFlags, ENUM_TEXTURE_FORMAT MyFormat, VkFormat Format, int FirstMip, int NumMips, int ArraySliceIndex, int NumArraySlices, bool bUseIdentitySwizzle);
	};
}

#endif //
