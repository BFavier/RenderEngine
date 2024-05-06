#pragma once
#include <vector>
#include <map>
#include <set>
#include <string>
#include <optional>
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/graphics/shaders/implementations/Shader3D.hpp>
#include <RenderEngine/graphics/shaders/implementations/DemoShader.hpp>

namespace RenderEngine
{
    class Window;

    class GPU
    {
    // A GPU is a piece of hardware to perform rendering calculations onto. There can be several GPUs on a single computer.

    friend class Internal;

    public:
        enum Type {DISCRETE_GPU=VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                   INTEGRATED_GPU=VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
                   VIRTUAL_GPU=VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
                   CPU=VK_PHYSICAL_DEVICE_TYPE_CPU,
                   UNKNOWN=VK_PHYSICAL_DEVICE_TYPE_OTHER};

    public: // This class is non copyable
        GPU() = delete;
        GPU(const GPU& other) = delete;
        GPU& operator=(const GPU& other) = delete;
        GPU(GPU&&) = default;
        GPU& operator=(GPU&&) = default;
    public:
        ~GPU();
    protected: // only the class RenderEngine::Internal can create GPUs
        GPU(VkPhysicalDevice device, const Window& window, const std::vector<const char*>& validation_layer_names, const std::vector<std::string>& extensions);
    public:
        // Device name
        std::string device_name() const;
        // Device constructor name
        std::string constructor_name() const;
        // Device total local memory in bytes
        uint64_t memory() const;
        // Returns the type of the device
        Type type() const;
        // Return the best supported depth format
        std::pair<VkImageTiling, VkFormat> depth_format() const;
        // Return whether dynamicaly changing face culling is supported
        bool dynamic_culling_supported() const;
    public:
        VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties _device_properties{};
        VkPhysicalDeviceFeatures _device_features{};
        VkPhysicalDeviceMemoryProperties _device_memory{};
        std::optional<std::tuple<uint32_t, VkQueue, VkCommandPool>> _graphics_queue; // (queue family, VkQueue, VkCOmmandPool)
        std::optional<std::tuple<uint32_t, VkQueue, VkCommandPool>> _compute_queue;
        std::optional<std::tuple<uint32_t, VkQueue, VkCommandPool>> _present_queue;
        std::set<std::string> _enabled_extensions;
        VkDevice _logical_device = VK_NULL_HANDLE;
        Shader3D* shader3d = nullptr;
        DemoShader* shader_draw_image = nullptr;
    protected:
        // returns the index of the queue family selected for 'queue_type' purpose. Modifies the 'selected_families_count'.
        std::optional<uint32_t> _select_queue_family(std::vector<VkQueueFamilyProperties>& queue_families,  // all available queue families
                                                     VkQueueFlagBits queue_type,  // the type of queue to create
                                                     std::map<uint32_t, uint32_t>& selected_families_count  // for each queue familly, tracks the number of queue that have already been selected
                                                     ) const;
        // select the index of the queue family selected for 'present' purprose. Modifies the 'selected_families_count'.
        std::optional<uint32_t> _select_present_queue_family(std::vector<VkQueueFamilyProperties>& queue_families,  // all available queue families
                                                             const Window& window, std::map<uint32_t, uint32_t>& selected_families_count,  // for each queue familly, tracks the number of queue that have already been selected
                                                             const std::optional<uint32_t>& graphics_family, // The previously selected graphics queue family index
                                                             bool& graphics_queue_is_present_queue // Whether the selected present queue is the graphics queue
                                                             ) const;
        // query the queue handle of a previously created queue
        std::optional<std::tuple<uint32_t, VkQueue, VkCommandPool>> _query_queue_handle(const std::optional<uint32_t>& queue_family,
                                                                                        std::map<uint32_t, uint32_t>& selected_families_count) const;
    protected:
        bool _dynamic_culling_supported;
    };
}
