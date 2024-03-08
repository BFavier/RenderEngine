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
        Buffer(const std::shared_ptr<GPU>& gpu, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        ~Buffer();
    public:
        const std::shared_ptr<GPU>& gpu;
        size_t size;
        std::shared_ptr<VkBuffer> _vk_buffer = nullptr;
        std::shared_ptr<VkDeviceMemory> _vk_memory = nullptr;
    public:
        void upload(const void* data, size_t length, size_t offset = 0);
        void download(void* data, size_t length, size_t offset) const;
    protected:
        uint32_t _find_memory_type(VkPhysicalDevice physical_device, uint32_t typeFilter, VkMemoryPropertyFlags properties);
        void _allocate_buffer(VkBufferUsageFlags usage);
        void _allocate_memory(VkMemoryPropertyFlags properties);
    };
}