#include <RenderEngine/graphics/GPU.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <set>
#include <utility>
#include <RenderEngine/user_interface/Handles.hpp>

using namespace RenderEngine;

GPU::GPU(VkPhysicalDevice device, const Handles& events, const std::vector<std::string>& extensions)
{
    // Save physical device
    _physical_device = device;
    // List properties and features
    vkGetPhysicalDeviceProperties(device, &_device_properties);
    vkGetPhysicalDeviceFeatures(device, &_device_features);
    vkGetPhysicalDeviceMemoryProperties(device, &_device_memory);
    // List the queue families
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());
    // list available extensions
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());
    std::vector<std::string> available_extension_names;
    for (VkExtensionProperties& properties : available_extensions)
    {
        available_extension_names.push_back(std::string(properties.extensionName));
    }
    // build list of extensions to enable
    std::vector<const char*> enabled_extensions;
    for (const std::string& extension_name : extensions)
    {
        if (std::find(available_extension_names.begin(), available_extension_names.end(), extension_name) != available_extension_names.end())
        {
            enabled_extensions.push_back(extension_name.c_str());
            _enabled_extensions.insert(extension_name);
        }
    }
    // Check if swap chain extension is supported
    bool swap_chain_supported = (_enabled_extensions.find(VK_KHR_SWAPCHAIN_EXTENSION_NAME) != _enabled_extensions.end());
    // Select the best matching queue families for each application
    std::map<uint32_t, uint32_t> selected_families_count; // number of purpose each queue is selected for
    std::optional<uint32_t> graphics_family = _select_queue_family(queue_families, VK_QUEUE_GRAPHICS_BIT, selected_families_count);
    std::optional<uint32_t> transfer_family = _select_queue_family(queue_families, VK_QUEUE_TRANSFER_BIT, selected_families_count);
    std::optional<uint32_t> compute_family = _select_queue_family(queue_families, VK_QUEUE_COMPUTE_BIT, selected_families_count);
    bool graphics_queue_is_present_queue = false;
    std::optional<uint32_t> present_family;
    if (swap_chain_supported)
    {
        present_family = _select_present_queue_family(queue_families, events, selected_families_count, graphics_family, graphics_queue_is_present_queue);
    }
    // Create logical device
    std::vector<std::vector<float>> priorities;
    std::vector<VkDeviceQueueCreateInfo> selected_families;
    for (std::pair<const uint32_t, uint32_t>& queue_family_count : selected_families_count)
    {
        priorities.push_back(std::vector(queue_family_count.second, 1.0f));
        VkDeviceQueueCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = queue_family_count.first;
        info.queueCount = queue_family_count.second;
        info.pQueuePriorities = priorities.back().data();
        selected_families.push_back(info);
    }
    VkDeviceCreateInfo device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.pQueueCreateInfos = selected_families.data();
    device_info.queueCreateInfoCount = selected_families.size();
    device_info.pEnabledFeatures = &_device_features;
    device_info.ppEnabledExtensionNames = enabled_extensions.data();
    device_info.enabledExtensionCount = enabled_extensions.size();
    VkDevice* logical_device = new VkDevice();
    VkResult result = vkCreateDevice(_physical_device, &device_info, nullptr, logical_device);
    _logical_device = std::shared_ptr<VkDevice>(logical_device, _deallocate_device);
    if (result != VK_SUCCESS)
    {
        THROW_ERROR("failed to create logical device")
    }
    // retrieve the queue handles
    _query_queue_handle(_graphics_family_queue, graphics_family, selected_families_count);
    _query_queue_handle(_transfer_family_queue, transfer_family, selected_families_count);
    _query_queue_handle(_compute_family_queue, compute_family, selected_families_count);
    if (graphics_queue_is_present_queue)
    {
        _present_family_queue = _graphics_family_queue;
    }
    else
    {
        _query_queue_handle(_present_family_queue, present_family, selected_families_count);
    }
}

GPU::GPU(const GPU& other)
{
    operator=(other);
}

GPU::~GPU()
{
}

std::string GPU::device_name() const
{
    return std::string(_device_properties.deviceName);
}

std::string GPU::constructor_name() const
{
    switch (_device_properties.vendorID)
    {
        case 0x1002:
            return std::string("AMD");
        case 0x1010:
            return std::string("ImgTec");
        case 0x10DE:
            return std::string("NVIDIA");
        case 0x13B5:
            return std::string("ARM");
        case 0x5143:
            return std::string("Qualcomm");
        case 0x8086:
            return std::string("Intel");
        default:
            return std::string("UNKNOWN");
    }
}

unsigned int GPU::memory() const
{
    uint32_t n_heaps = _device_memory.memoryHeapCount;
    unsigned int memory = 0;
    for (uint32_t i=0; i<n_heaps; i++)
    {
        VkMemoryHeap heap = _device_memory.memoryHeaps[i];
        if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        {
            memory += static_cast<unsigned int>(heap.size);
        }
    }
    return memory;
}

GPU::Type GPU::type() const
{
    switch (_device_properties.deviceType)
    {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return INTEGRATED_GPU;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return DISCRETE_GPU;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return VIRTUAL_GPU;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return CPU;
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            return UNKNOWN;
        default:
            return UNKNOWN;
    }
}

std::vector<GPU> GPU::get_devices()
{

    VkInstance instance = Engine::get_vulkan_instance();
    // Get the number of devices
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    // Get all the devices
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
    // Create the dummy window
    WindowSettings settings;
    settings.visible = false;
    Handles events(settings);
    // Return the GPU objects
    std::vector<GPU> GPUs;
    for(VkPhysicalDevice& device : devices)
    {
        GPUs.push_back(GPU(device, events));
    }
    return GPUs;
}

GPU GPU::get_best_device()
{
    // list the available GPUs and split them by type
    std::vector<GPU> GPUs = get_devices();
    std::vector<GPU> discrete_GPUs;
    std::vector<GPU> other_GPUs;
    for (GPU& gpu : GPUs)
    {
        if (gpu.type() == GPU::Type::DISCRETE_GPU)
        {
            discrete_GPUs.push_back(gpu);
        }
        else
        {
            other_GPUs.push_back(gpu);
        }
    }
    // chose the best available subset of GPUs
    std::vector<GPU> subset;
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
        THROW_ERROR("No GPU found that match the criteria")
    }
    // select the GPU with most memory
    unsigned int max_memory = 0;
    unsigned int i_max = 0;
    for (unsigned int i=0; i<subset.size(); i++)
    {
        GPU gpu = subset[i];
        unsigned int memory = gpu.memory();
        if (memory > max_memory)
        {
            i_max = i;
            max_memory = memory;
        }
    }
    return subset[i_max];
}

void GPU::_deallocate_device(const VkDevice* device)
{
    vkDeviceWaitIdle(*device);
    vkDestroyDevice(*device, nullptr);
    delete device;
}

void GPU::operator=(const GPU& other)
{
    _physical_device = other._physical_device;
    _device_properties = other._device_properties;
    _device_features = other._device_features;
    _device_memory = other._device_memory;
    _graphics_family_queue = other._graphics_family_queue;
    _compute_family_queue = other._compute_family_queue;
    _transfer_family_queue = other._transfer_family_queue;
    _present_family_queue = other._present_family_queue;
    _enabled_extensions = other._enabled_extensions;
    _logical_device = other._logical_device;
}

std::optional<uint32_t> GPU::_select_queue_family(std::vector<VkQueueFamilyProperties>& queue_families,
                                                  VkQueueFlagBits queue_type,
                                                  std::map<uint32_t, uint32_t>& selected_families_count) const
{
    std::vector<VkQueueFlagBits> functionalities = {VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT};
    // filter families that handle the operation type
    std::map<uint32_t, VkQueueFamilyProperties> valid_families;
    for (uint32_t i=0; i<queue_families.size(); i++)
    {
        VkQueueFamilyProperties& queue_family = queue_families[i];
        if ((queue_family.queueFlags & queue_type) &&
            (queue_family.queueCount > 0))
        {
            valid_families[i] = queue_family;
        }
    }
    // Looking for the most specialized queue
    std::optional<uint32_t> selected_family;
    unsigned int min_n_functionalities;
    for (std::pair<const uint32_t, VkQueueFamilyProperties>& family : valid_families)
    {
        // if there is no selected family yet, set current family and number of functionalities
        if (!selected_family.has_value())
        {
            selected_family = family.first;
            VkQueueFlags flags = family.second.queueFlags;
            min_n_functionalities = 0;
            for (VkQueueFlagBits bits : functionalities)
            {
                min_n_functionalities += (flags & bits) / bits;
            }
        }
        // otherwise change selected family only if it has less functionalities
        else
        {
            VkQueueFlags flags = family.second.queueFlags;
            unsigned int n_functionalities = 0;
            for (VkQueueFlagBits bits : functionalities)
            {
                n_functionalities += (flags & bits) / bits;
            }
            if (n_functionalities < min_n_functionalities)
            {
                selected_family = family.first;
            }
        }
    }
    // if a family was selected decrement its number of available queues, and increment its selected count
    if (selected_family.has_value())
    {
        queue_families[selected_family.value()].queueCount -= 1;
        if (selected_families_count.find(selected_family.value()) != selected_families_count.end())
        {
            selected_families_count[selected_family.value()] += 1;
        }
        else
        {
            selected_families_count[selected_family.value()] = 1;
        }
    }
    // return the selected family if there was any
    return selected_family;
}

std::optional<uint32_t> GPU::_select_present_queue_family(std::vector<VkQueueFamilyProperties>& queue_families,
                                                          const Handles& events, std::map<uint32_t, uint32_t>& selected_families_count,
                                                          const std::optional<uint32_t>& graphics_family, bool& graphics_queue_is_present_queue) const
{
    std::optional<uint32_t> queue_family;
    graphics_queue_is_present_queue = false;
    // Check if the graphic queue can be the present queue
    if (graphics_family.has_value())
    {
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(_physical_device, graphics_family.value(), events._vk_surface, &present_support);
        if (present_support)
        {
            graphics_queue_is_present_queue = true;
            return graphics_family;
        }
    }
    // Otherwise look up for a queue family that can handle presenting to a window
    for (uint32_t i=0; i<queue_families.size(); i++)
    {
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(_physical_device, i, events._vk_surface, &present_support);
        if (present_support && queue_families[i].queueCount > 0)
        {
            queue_family = i;
            queue_families[i].queueCount -= 1;
            selected_families_count[i] += 1;
            break;
        }
    }
    return queue_family;
}

void GPU::_query_queue_handle(std::optional<std::pair<uint32_t, VkQueue>>& queue,
                              const std::optional<uint32_t>& queue_family,
                              std::map<uint32_t, uint32_t>& selected_families_count) const
{
    if (queue_family.has_value())
    {
        uint32_t family = queue_family.value();
        VkQueue queried_queue;
        vkGetDeviceQueue(*_logical_device, family, selected_families_count[family]-1, &queried_queue);
        queue = std::make_pair(family, queried_queue);
        selected_families_count[family] -= 1;
    }
}