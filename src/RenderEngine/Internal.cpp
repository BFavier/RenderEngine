#include <RenderEngine/Internal.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <RenderEngine/graphics/Image.hpp>
#include <RenderEngine/user_interface/WindowSettings.hpp>
#include <RenderEngine/user_interface/Window.hpp>

using namespace RenderEngine;

void (*vkCmdPushDescriptorSet)(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites);
bool Internal::_initialized = false;
VkInstance Internal::_vk_instance;
VkDebugUtilsMessengerEXT Internal::_debug_messenger;
std::vector<std::unique_ptr<GPU>> Internal::GPUs;

void Internal::initialize(const std::vector<std::string>& validation_layers,
                          const std::vector<std::string>& instance_extensions,
                          const std::vector<std::string>& device_extensions)
{
    if (_initialized)
    {
        return;
    }
    _initialized = true;
    //Initialize GLFW
    if (!glfwInit())
    {
        THROW_ERROR("Failed to initialize the library GLFW");
    }
    // Set application name
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "RenderEngine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Internal";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
    // verifies that requested validation layers are supported
    std::vector<std::string> available_validation_layers = get_available_validation_layers();
    std::vector<const char*> validation_layer_names;
    for (const std::string& layer_name : validation_layers)
    {
        if (std::find(available_validation_layers.begin(), available_validation_layers.end(), layer_name) == available_validation_layers.end())
        {
            std::string error_message = "Unsupported validation layer: '" + layer_name + "'";
            THROW_ERROR(error_message);
        }
        validation_layer_names.push_back(layer_name.c_str());
    }
    // list extensions to activate (in addition to the ones requested by user, some are required by glfw)
    std::vector<const char*> _extensions;
    for (const std::string& ext : instance_extensions)
    {
        _extensions.push_back(ext.c_str());
    }
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (uint32_t i=0; i<glfwExtensionCount; i++)
    {
        _extensions.push_back(glfwExtensions[i]);
    }
    // Set validation layers callback function
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {};
    debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_create_info.pfnUserCallback = _debug_callback;
    debug_create_info.pUserData = nullptr;
    // Create the VkInstance
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = validation_layer_names.size();
    createInfo.ppEnabledLayerNames = validation_layer_names.data();
    createInfo.pNext = &debug_create_info;
    createInfo.enabledExtensionCount = _extensions.size();
    createInfo.ppEnabledExtensionNames = _extensions.data();
    VkResult result = vkCreateInstance(&createInfo, nullptr, &_vk_instance);
    if (result != VK_SUCCESS)
    {
        THROW_ERROR("Failed to create the Vulkan instance");
    }
    // setup validation layers callback function
    if (_create_debug_utils_messenger_EXT(_vk_instance, &debug_create_info, nullptr, &_debug_messenger) != VK_SUCCESS)
    {
        THROW_ERROR("Failed to set up debug messenger");
    }
    // fill up the GPUs
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(_vk_instance, &device_count, nullptr);
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(_vk_instance, &device_count, devices.data());
    WindowSettings settings;
    settings.title = "DummyWindow";
    settings.width = 5;
    settings.height = 5;
    settings.visible = false;
    settings.initialize_swapchain = false;
    Window dummy_window(settings);
    for(VkPhysicalDevice& device : devices)
    {
        GPUs.emplace_back(new GPU(device, dummy_window, validation_layer_names, device_extensions));
        GPU* gpu = GPUs.back().get();
        gpu->_default_textures = Image::bulk_allocate_images(gpu, 1, ImageFormat::RGBA, 2, 2, false);
        for (std::shared_ptr<Image>& image : gpu->_default_textures)
        {
            image->upload_data({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
        }
    }
    // Load extension functions
    vkCmdPushDescriptorSet = reinterpret_cast<PFN_vkCmdPushDescriptorSetKHR>(vkGetInstanceProcAddr(_vk_instance, "vkCmdPushDescriptorSetKHR"));
    if (vkCmdPushDescriptorSet == nullptr)
    {
        THROW_ERROR("Failed to load extension function ")
    }
}

std::vector<const GPU*> Internal::get_detected_GPUs()
{
    Internal::initialize();
    std::vector<const GPU*> _gpus;
    for (const std::unique_ptr<GPU>& gpu : GPUs)
    {
        _gpus.push_back(gpu.get());
    }
    return _gpus;
}

const GPU* Internal::get_best_GPU()
{
    Internal::initialize();
    if (GPUs.size() == 0)
    {
        THROW_ERROR("No GPU available on current machine.");
    }
    // list the available GPUs and split them by type
    std::vector<const GPU*> discrete_GPUs;
    std::vector<const GPU*> other_GPUs;
    for (const std::unique_ptr<GPU>& gpu : GPUs)
    {
        if (gpu->type() == GPU::Type::DISCRETE_GPU)
        {
            discrete_GPUs.push_back(gpu.get());
        }
        else
        {
            other_GPUs.push_back(gpu.get());
        }
    }
    // chose the best available subset of GPUs
    std::vector<const GPU*> subset;
    if (discrete_GPUs.size() > 0)
    {
        subset = discrete_GPUs;
    }
    else if (other_GPUs.size() > 0)
    {
        subset = other_GPUs;
    }
    else
    {
        THROW_ERROR("No GPU found that match the criteria");
    }
    // select the GPU with most memory
    unsigned int max_texture_size = 0;
    const GPU* best = nullptr;
    for (const GPU* gpu : subset)
    {
        unsigned int texture_size = gpu->memory();
        if (texture_size > max_texture_size)
        {
            best = gpu;
            max_texture_size = texture_size;
        }
    }
    return best;
}

void Internal::terminate()
{
    _initialized = false;
    GPUs.clear();
    //Terminate Vulkan
    _destroy_debug_utils_messenger_EXT(_vk_instance, _debug_messenger, nullptr);
    vkDestroyInstance(_vk_instance, nullptr);
    //Terminate GLFW
    glfwTerminate();
}

std::vector<std::string> Internal::get_available_validation_layers()
{
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
    std::vector<std::string> layer_names;
    for (const VkLayerProperties& layer : available_layers)
    {
        layer_names.push_back(std::string(layer.layerName));
    }
    return layer_names;
}

std::vector<std::string> Internal::get_available_vulkan_extensions()
{
    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());
    std::vector<std::string> available_extensions;
    for (VkExtensionProperties& extension : extensions)
    {
        available_extensions.push_back(std::string(extension.extensionName));
    }
    return available_extensions;
}

VkInstance Internal::get_vulkan_instance()
{
    Internal::initialize();
    return _vk_instance;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Internal::_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                       void* pUserData)
{
    //silencing unused variables
    (void)pCallbackData;
    (void)pUserData;
    (void)messageType;

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        std::cerr << "[validation layer]" << std::endl;
        std::cerr << pCallbackData->pMessage << std::endl;
    }
    return VK_FALSE;
}

VkResult Internal::_create_debug_utils_messenger_EXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Internal::_destroy_debug_utils_messenger_EXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}
