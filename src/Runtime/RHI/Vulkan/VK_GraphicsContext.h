#pragma once

#ifndef _VK_GRAPHICSCONTEXT_
#define _VK_GRAPHICSCONTEXT_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include"../GraphicsContext.h"
#include <string>
#include<vector>
#include<memory>
#include <optional>
#include "vulkan/vulkan_core.h"

namespace MXRender { class VK_RenderPass; }

namespace MXRender { class VK_DescriptorPool; }

namespace MXRender { class RenderPass; }



namespace MXRender
{
    class VK_Device;
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class VK_GraphicsContext:public GraphicsContext
    {
    private:
        void create_instance();
        void initialize_debugmessenger();
        void create_surface(GLFWwindow* window);
        void initialize_physical_device();
        void create_logical_device();
        void create_command_pool();
        void create_command_buffer();
        void create_sync_object();

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
        std::shared_ptr<VK_Device> device;

        VkQueue graphicsQueue;
        VkQueue presentQueue;
 
        GLFWwindow* window{ nullptr };

        static uint8_t const max_frames_in_flight{ 3 };
        uint8_t   current_frame_index{ 0 };
        VkCommandPool command_pool[max_frames_in_flight];
        std::vector< VkCommandBuffer> command_buffer;//[max_frames_in_flight];
		VkSemaphore          image_available_for_render_semaphore[max_frames_in_flight];
		VkSemaphore          image_finished_for_presentation_semaphore[max_frames_in_flight];
		VkFence              frame_in_flight_fence[max_frames_in_flight];

        uint32_t current_swapchain_image_index=0;

		VkSwapchainKHR           swapchain{ VK_NULL_HANDLE };
		VkFormat                 swapchain_image_format{ VK_FORMAT_UNDEFINED };
        VkFormat                 depth_image_format{ VK_FORMAT_UNDEFINED };
		VkExtent2D               swapchain_extent;
		std::vector<VkImage>     swapchain_images;
		
		VkViewport       viewport;
		VkRect2D         scissor;

		VkImage        depth_image{ VK_NULL_HANDLE };
		VkDeviceMemory depth_image_memory{ VK_NULL_HANDLE };
		VkImageView    depth_image_view{ VK_NULL_HANDLE };

        bool framebufferResized = false;
        //std::shared_ptr <VK_DescriptorPool> descriptor_pool;
    public:
        VK_GraphicsContext();
        virtual ~VK_GraphicsContext();
        virtual void init(GLFWwindow* window) override;
        virtual void pre_init() override;

        std::vector<VkImageView> swapchain_imageviews;

        std::shared_ptr<VK_Device> get_device();
        //std::shared_ptr<VK_DescriptorPool> get_descriptor_pool();
        VkInstance get_instance();

        void wait_for_fences();
        void reset_commandbuffer();

        void create_swapchain();
        void recreate_swapchain();
        void clean_swapchain();
		void create_swapchain_imageviews();
		void create_framebuffer_imageAndview();

        VkSurfaceKHR get_surface();
        VkCommandBuffer get_cur_command_buffer() ;
        uint8_t get_current_frame_index() const;
        uint32_t get_current_swapchain_image_index() const;
        VkFormat get_swapchain_image_format() const;
        VkExtent2D get_swapchain_extent() const;
        void copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);

        void pre_pass( );
        void submit( );
    };
}
#endif

