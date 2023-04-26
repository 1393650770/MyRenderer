#pragma once

#ifndef _VK_GRAPHICSCONTEXT_
#define _VK_GRAPHICSCONTEXT_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>

#include"../GraphicsContext.h"
#include <string>
#include<vector>
#include<memory>
#include <optional>
#include <unordered_map>
#include <functional>

#include "../../Render/Pass/PipelineShaderObject.h"

#include <queue>
#include <future>

#include "../../../ThirdParty/vma/vk_mem_alloc.h"

namespace MXRender { struct MipmapInfo; }

namespace MXRender { class AllocatedImage; }

namespace MXRender { class AllocatedBufferUntyped; }

namespace MXRender { class MeshBase; }

namespace MXRender { class VK_RenderPass; }

namespace MXRender { class VK_DescriptorPool; }

namespace MXRender { class RenderPass; }

namespace MXRender {class MaterialSystem;} 

namespace MXRender
{

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        "VK_EXT_sampler_filter_minmax"
	};

    class VK_Device;
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> computeFamily;
        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cerr << "validation layer: " << messageSeverity<<" "<< messageType<<" " << pCallbackData->pMessage << std::endl;
        if (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT==messageSeverity&& messageType== VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
        {
            std::abort();
        }
		return VK_FALSE;
	}

    class VK_GraphicsContext:public GraphicsContext
    {
    private:
        void create_instance();
        void initialize_debugmessenger();
        void create_surface();
        void initialize_physical_device();
        void create_logical_device();
        void init_vma_allocator();
        void create_command_pool();
        void create_command_buffer();
        void create_sync_object();

        void create_depth_pyramid_image();

        VkResult create_debug_utils_messengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
        void destroy_debug_utils_messengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
        std::vector<const char*> get_required_extensions();
        bool check_validationlayer_support();
        void populate_debug_messenger_createInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        bool check_device_suitable(VkPhysicalDevice device);
        QueueFamilyIndices find_queue_families(VkPhysicalDevice device);
        bool check_device_extension_support(VkPhysicalDevice device);
        SwapChainSupportDetails query_swapchain_support(VkPhysicalDevice device);
		VkFormat find_supported_depth_format(const std::vector<VkFormat>& candidates,
			VkImageTiling                tiling,
			VkFormatFeatureFlags         features);

		VkSurfaceFormatKHR
			chooseSwapchainSurfaceFormatFromDetails(const std::vector<VkSurfaceFormatKHR>& available_surface_formats);
		VkPresentModeKHR
			chooseSwapchainPresentModeFromDetails(const std::vector<VkPresentModeKHR>& available_present_modes);
		VkExtent2D chooseSwapchainExtentFromDetails(const VkSurfaceCapabilitiesKHR& capabilities);
    
    protected:
        VkInstance instance;
        VkDebugUtilsMessengerEXT debug_messenger;
        VkSurfaceKHR surface;


    

        GLFWwindow* window{ nullptr };

        static uint8_t const max_frames_in_flight{ 3 };
        uint8_t   current_frame_index{ 0 };
        
        std::vector< VkCommandBuffer> command_buffer;//[max_frames_in_flight];
        std::vector< VkCommandBuffer> thread_command_buffer;

		VkSemaphore          image_available_for_render_semaphore[max_frames_in_flight];
		VkSemaphore          image_finished_for_presentation_semaphore[max_frames_in_flight];
		VkFence              frame_in_flight_fence[max_frames_in_flight];

        uint32_t current_swapchain_image_index=-1;

		VkSwapchainKHR           swapchain{ VK_NULL_HANDLE };
		VkFormat                 swapchain_image_format{ VK_FORMAT_UNDEFINED };

        VkFormat                 depth_image_format{ VK_FORMAT_UNDEFINED };
		VkExtent2D               swapchain_extent;
		std::vector<VkImage>     swapchain_images;
		VkRect2D         scissor;

		VkImage        depth_image{ VK_NULL_HANDLE };
		VkDeviceMemory depth_image_memory{ VK_NULL_HANDLE };
		VkImageView    depth_image_view{ VK_NULL_HANDLE };


        bool framebufferResized = false;

        std::vector<std::function<void()>>  on_swapchain_recreate;
        std::vector<std::function<void()>>  on_swapchain_clean;
        std::vector<std::function<void()>>  on_shutdown_clean;
        //std::shared_ptr <VK_DescriptorPool> descriptor_pool;
    public:
        VkViewport       viewport;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
        VkQueue computeQueue;
        VkRenderPass mesh_pass;
        VkDescriptorPool descriptor_pool;
        VkCommandPool command_pool;//[max_frames_in_flight];
        std::vector< VkCommandPool> thread_command_pool;
        std::unordered_map<VkRenderPass, VkCommandBufferInheritanceInfo> inheritance_info_map;
        std::unordered_map<VkRenderPass, VkRenderPassBeginInfo> renderpass_begin_info_map;
        std::unordered_map<VkCommandBuffer, unsigned int> thread_command_buffer_use_map;

        std::queue<std::future<void>> fut_que;

		AllocatedImage depth_pyramid_image;
		int depth_pyramid_width;
		int depth_pyramid_height;
		int depth_pyramid_levels;
		VkSampler depth_sampler=VK_NULL_HANDLE;
		VkImageView depth_pyramid_mips[16] = {};

        VmaAllocator _allocator;
        VK_GraphicsContext();
        virtual ~VK_GraphicsContext();
        virtual void init(Window* new_window) override;
        virtual void pre_init() override;
        std::vector<VkFramebuffer*> swapchain_framebuffers;
        std::vector<VkImageView> swapchain_imageviews;

        std::shared_ptr<VK_Device> device;
        //std::shared_ptr<VK_DescriptorPool> get_descriptor_pool();
        VkInstance get_instance();

        void wait_for_fences();
        void reset_commandpool();

        void create_swapchain();
        void recreate_swapchain();
        void clean_swapchain();
		void create_swapchain_imageviews();
		void create_framebuffer_imageAndview();

        VkSurfaceKHR get_surface();
        VkCommandBuffer get_cur_command_buffer() ;
        VkCommandBuffer get_cur_threadid_command_buffer(unsigned int thread_id);
        void reset_all_threadid_command_buffer();
        void begin_all_threadid_command_buffer();
        void end_all_threadid_command_buffer();
        void execute_all_threadid_command_buffer();
        void submit_all_threadid_command_buffer();
        void wait_all_task();
        uint8_t get_current_frame_index() const;
        uint8_t get_max_frame_num() const;
        uint32_t get_current_swapchain_image_index() const;
        VkFormat get_swapchain_image_format() const;
        VkFormat get_depth_image_format() const;
        VkExtent2D get_swapchain_extent() const;
        VkImageView get_depth_image_view() const;
        VkImage get_depth_image() const;
        VkImage get_cur_swapchain_image();
        GLFWwindow* get_window() const;
        QueueFamilyIndices get_queuefamily() ;
        void copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
        VkCommandBuffer begin_single_time_commands();
        void end_single_time_commands(VkCommandBuffer command_buffer);

        void add_on_swapchain_recreate_func(const std::function<void()>& func);
        void add_on_swapchain_clean_func(const std::function<void()>& func);
        void add_on_shutdown_clean_func(const std::function<void()>& func);
        void pre_pass( );
        void init_pass();
        void submit();
        void cleanup();
		AllocatedBufferUntyped create_allocate_buffer( size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags required_flags);
		AllocatedImage upload_allocate_image(VK_GraphicsContext* context, int texWidth, int texHeight, VkFormat image_format, AllocatedBufferUntyped& stagingBuffer, std::vector<MipmapInfo> mips);
        void* map_allocate_buffer(AllocatedBufferUntyped& buffer);
        void unmap_allocate_buffer(AllocatedBufferUntyped& buffer);
        void destroy_allocate_buffer(AllocatedBufferUntyped& buffer);
    };
}
#endif
