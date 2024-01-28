#pragma once
#include <vector>
#include <map>
#include <set>
#include <string>
#include <optional>
#include <RenderEngine/utilities/External.hpp>

namespace RenderEngine
{
    class Window;

    class GPU
    {
    // A GPU is a piece of hardware to perform rendering calculations onto. There can be several GPUs on a single computer.

    public:
        enum Type {DISCRETE_GPU=VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                INTEGRATED_GPU=VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
                VIRTUAL_GPU=VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
                CPU=VK_PHYSICAL_DEVICE_TYPE_CPU,
                UNKNOWN=VK_PHYSICAL_DEVICE_TYPE_OTHER};

    public:
        GPU() = delete;
        GPU(VkPhysicalDevice device, const Window& window, const std::vector<std::string>& extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME});
        ~GPU();
    public:
        // Device name
        std::string device_name() const;
        // Device constructor name
        std::string constructor_name() const;
        // Device total local memory in bytes
        uint64_t memory() const;
        // Returns the type of the device
        Type type() const;
    public:
        VkPhysicalDevice _physical_device;
        VkPhysicalDeviceProperties _device_properties;
        VkPhysicalDeviceFeatures _device_features;
        VkPhysicalDeviceMemoryProperties _device_memory;
        std::optional<std::pair<uint32_t, VkQueue>> _graphics_family_queue;
        std::optional<std::pair<uint32_t, VkQueue>> _transfer_family_queue;
        std::optional<std::pair<uint32_t, VkQueue>> _compute_family_queue;
        std::optional<std::pair<uint32_t, VkQueue>> _present_family_queue;
        std::set<std::string> _enabled_extensions;
        VkDevice _logical_device;
    protected:
        // add a queue family of given type to the selected families
        std::optional<uint32_t> _select_queue_family(std::vector<VkQueueFamilyProperties>& queue_families,
                                                     VkQueueFlagBits queue_type,
                                                     std::map<uint32_t, uint32_t>& selected_families_count) const;
        // select the present queue family specificaly
        std::optional<uint32_t> _select_present_queue_family(std::vector<VkQueueFamilyProperties>& queue_families,
                                                             const Window& window, std::map<uint32_t, uint32_t>& selected_families_count,
                                                             const std::optional<uint32_t>& graphics_family, bool& graphics_queue_is_present_queue) const;
        // query the queue handle of a previously created queue
        void _query_queue_handle(std::optional<std::pair<uint32_t, VkQueue>>& queue,
                                 const std::optional<uint32_t>& queue_family,
                                 std::map<uint32_t, uint32_t>& selected_families_count) const;
    };
}
