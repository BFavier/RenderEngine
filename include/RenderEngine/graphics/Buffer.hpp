#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/graphics/GPU.hpp>

namespace RenderEngine
{
    class Buffer
    // A buffer is an array of memory visible both by CPU and GPU, used to transfer data to and from GPU stored objects
    {
    public:
        Buffer() = delete;
        Buffer(const std::shared_ptr<GPU>& gpu, size_t bytes_size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties);
        ~Buffer();
    public:
        const std::shared_ptr<GPU>& gpu;
        std::shared_ptr<VkBuffer> _vk_buffer = nullptr;
        std::shared_ptr<VkDeviceMemory> _vk_memory = nullptr;
    public:
        size_t bytes_size() const;
        void upload(const void* data) const;
        void download(void* data) const;
    protected:
        void* _data = nullptr;
        size_t _bytes_size = 0;
        VkMemoryPropertyFlags _memory_properties = 0;
    protected:
        uint32_t _find_memory_type(VkPhysicalDevice physical_device, uint32_t typeFilter, VkMemoryPropertyFlags memory_properties);
        void _allocate_buffer(VkBufferUsageFlags usage);
        void _allocate_memory(VkMemoryPropertyFlags memory_properties);
    };
}