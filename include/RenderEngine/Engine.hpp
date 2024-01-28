#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>

namespace RenderEngine
{
    class GPU;

    class Engine
    {
    public:
        Engine() = delete;
    public:
        //Initialize the used libraries
        static void initialize(const std::vector<std::string>& validation_layers={});
        static void terminate();
        static std::vector<std::string> get_available_validation_layers();
        static std::vector<std::string> get_available_vulkan_extensions();
        static VkInstance get_vulkan_instance();
        static const std::vector<GPU>& get_detected_GPUs();
        static const GPU& get_best_GPU();
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
        static std::vector<GPU> GPUs;
    };
}
