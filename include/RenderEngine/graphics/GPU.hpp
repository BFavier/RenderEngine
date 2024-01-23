#pragma once
#include <vector>
#include <map>
#include <set>
#include <string>
#include <optional>
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/Engine.hpp>

namespace RenderEngine
{
    class Window;
    class Handles;

    class GPU
    {

    enum Type {INTEGRATED_GPU, DISCRETE_GPU, VIRTUAL_GPU, CPU, UNKNOWN};

    public:
        GPU() = delete;
        GPU(VkPhysicalDevice device, const Handles& events, const std::vector<std::string>& extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME});
        GPU(const GPU& other);
        ~GPU();
        // Device name
        std::string device_name() const;
        // Device constructor name
        std::string constructor_name() const;
        // Device total local memory in bytes
        unsigned int memory() const;
        // Returns the type of the device
        Type type() const;
    public:
        static std::vector<GPU> get_devices();
        static GPU get_best_device();
    public:
        void operator=(const GPU& other);
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
                                                             const Handles& events, std::map<uint32_t, uint32_t>& selected_families_count,
                                                             const std::optional<uint32_t>& graphics_family, bool& graphics_queue_is_present_queue) const;
        // query the queue handle of a previously created queue
        void _query_queue_handle(std::optional<std::pair<uint32_t, VkQueue>>& queue,
                                 const std::optional<uint32_t>& queue_family,
                                 std::map<uint32_t, uint32_t>& selected_families_count) const;
    };
}
