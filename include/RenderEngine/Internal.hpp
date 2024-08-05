#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>

// function pointer definition. This function is an extension and must be loaded with vkGetInstanceProcAddr at library loading time
extern void (*vkCmdPushDescriptorSet)(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites);

namespace RenderEngine
{
    class GPU;

    class Internal
    {
    public:
        Internal() = delete;
    public:
        //Initialize the used libraries
        static void initialize(const std::vector<std::string>& validation_layers={}, // For debug, add "VK_LAYER_KHRONOS_validation" to enable validation layer (slower than if they are disabled)
                               const std::vector<std::string>& instance_extensions={VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                               const std::vector<std::string>& device_extensions={VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME});
        static void terminate();
        static std::vector<std::string> get_available_validation_layers();
        static std::vector<std::string> get_available_vulkan_extensions();
        static VkInstance get_vulkan_instance();
        static std::vector<const GPU*> get_detected_GPUs();
        static const GPU* get_best_GPU();
    protected:
        static VKAPI_ATTR VkBool32 VKAPI_CALL _debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                              VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                              void* pUserData);
        static VkResult _create_debug_utils_messenger_EXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
        static void _destroy_debug_utils_messenger_EXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    protected:
        ///< If true, the game engine was already initialized
        static bool _initialized;
        static VkInstance _vk_instance;
        static VkDebugUtilsMessengerEXT _debug_messenger;
        static std::vector<std::unique_ptr<GPU>> GPUs;
    };
}
