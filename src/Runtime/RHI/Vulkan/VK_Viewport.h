#pragma once

#ifndef _VK_VIEWPORT_
#define _VK_VIEWPORT_
#include "RHI/RenderViewport.h"

#include "vulkan/vulkan_core.h"




MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

class VulkanRHI;
class VK_SwapChain;
class VK_Device; 
struct VK_SwapChainRecreateInfo;
struct VK_TextureView; 
class VK_Texture;
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_Viewport,public Viewport)
#pragma region METHOD
public:
	VK_Viewport(VulkanRHI* in_rhi, VK_Device* in_device, void* in_window_handle, UInt32 in_size_x, UInt32 in_size_y, Bool in_is_full_screen,CONST  ENUM_TEXTURE_FORMAT& in_pixel_format);
	VIRTUAL ~VK_Viewport();

	VIRTUAL Vector<Texture*> METHOD(GetCurrentBackBufferRTV)() OVERRIDE FINAL;
	VIRTUAL Texture* METHOD(GetCurrentBackBufferDSV)() OVERRIDE FINAL;
	VIRTUAL Vector<UInt32> METHOD(GetViewportSize)() CONST OVERRIDE FINAL;
	VIRTUAL UInt32 METHOD(GetViewportSizeWidth)() CONST OVERRIDE FINAL;
	VIRTUAL UInt32 METHOD(GetViewportSizeHeight)() CONST OVERRIDE FINAL;

protected:
	void METHOD(CreateSwapChain)(VK_SwapChainRecreateInfo* recreate_info=nullptr);
private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	VK_Device* device = nullptr;
	VulkanRHI* rhi = nullptr;
	UInt32 size_x = 0;
	UInt32 size_y = 0;
	bool is_full_screen = false;
	ENUM_TEXTURE_FORMAT common_pixel_format;
	VkFormat vk_pixel_format;
	UInt32 acquired_image_index =0 ;
	VK_SwapChain* swap_chain =nullptr ;
	void* window_handle = nullptr ;
	UInt32 present_count = 0 ;
	bool render_offscreen = false ;
	Int8 lock_to_sync=0;

	Vector<VK_TextureView> texture_views;
	Vector<VK_Texture*> back_buffer_rtvs;
	VK_Texture* back_buffer_dsv = nullptr;
private:
#pragma endregion


MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif
