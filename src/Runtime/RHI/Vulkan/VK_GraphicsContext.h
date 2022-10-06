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

    const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
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

    protected:
        VkInstance instance;
        VkDebugUtilsMessengerEXT debug_messenger;
        VkSurfaceKHR surface;
        std::shared_ptr<VK_Device> device;

        VkQueue graphicsQueue;
        VkQueue presentQueue;
        
        VkCommandPool command_pool;
        VkCommandBuffer command_buffer;
		VkSemaphore          image_available_for_render_semaphore;
		VkSemaphore          image_finished_for_presentation_semaphore;
		VkFence              frame_in_flight_fence;
    public:
        VK_GraphicsContext();
        virtual ~VK_GraphicsContext();
        virtual void init(GLFWwindow* window) override;
        virtual void pre_init() override;

        std::shared_ptr<VK_Device> get_device();
        VkInstance get_instance();

        void wait_for_fences();
        void reset_commandbuffer();
        VkSurfaceKHR get_surface();
        void copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
        void pre_pass(VkSwapchainKHR& swapchain, uint32_t& image_index);
       
        void draw(std::vector<RenderPass*> render_pass,RenderPass* return_pass);
        void submit(VkSwapchainKHR* swapchains, int swapchain_count, uint32_t& image_index);
    };
}
#endif

