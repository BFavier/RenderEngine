#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <memory>

namespace RenderEngine
{
    class Buffer
    // A buffer is an array of GPU memory that can be synchronized with CPU RAM
    {
        friend class Canvas;
        friend class Image;
    public: // This class is non copyable
        Buffer() = delete;
        Buffer(const Buffer& other) = delete;
        Buffer& operator=(const Buffer& other) = delete;
    public:
        Buffer(const std::shared_ptr<GPU>& gpu, size_t bytes_size, VkBufferUsageFlags usage);
        ~Buffer();
    public:
        const std::shared_ptr<GPU>& gpu;
    public:
        std::size_t bytes_size() const;
        void upload(const uint8_t* data, std::size_t bytes_size, std::size_t offset) const;
        void download(uint8_t* data, std::size_t bytes_size, std::size_t offset) const;
    protected:
        VkBuffer _vk_buffer = VK_NULL_HANDLE;
        VkDeviceMemory _vk_memory = VK_NULL_HANDLE;
        uint8_t* _data = nullptr;
        std::size_t _bytes_size = 0;
        VkMemoryPropertyFlags _memory_properties = 0;
    protected:
        void _allocate_buffer(VkBufferUsageFlags usage);
        void _allocate_memory(VkMemoryPropertyFlags memory_properties);
    };
}