#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/graphics/GPU.hpp>

namespace RenderEngine
{
    class Buffer
    // A buffer is an array of GPU memory that can be synchronized with CPU RAM
    {
        friend class Canvas;
        friend class Image;
    public:
        Buffer() = delete;
        Buffer(const Buffer& other) = delete;
        Buffer& operator=(const Buffer& other) = delete;
    public:
        Buffer(const std::shared_ptr<GPU>& gpu, size_t bytes_size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties);
        ~Buffer();
    public:
        const std::shared_ptr<GPU>& gpu;
    public:
        size_t bytes_size() const;
        void upload(const void* data) const;
        void download(void* data) const;
    protected:
        VkBuffer* _vk_buffer = nullptr;
        VkDeviceMemory* _vk_memory = nullptr;
        void* _data = nullptr;
        size_t _bytes_size = 0;
        VkMemoryPropertyFlags _memory_properties = 0;
    protected:
        uint32_t _find_memory_type(VkPhysicalDevice physical_device, uint32_t typeFilter, VkMemoryPropertyFlags memory_properties);
        void _allocate_buffer(VkBufferUsageFlags usage);
        void _allocate_memory(VkMemoryPropertyFlags memory_properties);
    };
}