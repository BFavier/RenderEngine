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
        Buffer(const GPU& gpu, VkDeviceSize size);
        ~Buffer();
    public:
        const GPU* gpu;
        VkDeviceSize size;
        VkBuffer _buffer;
        VkDeviceMemory _buffer_memory;
    public:
        void upload(const unsigned char* data, size_t length, size_t offset = 0);
        void download(unsigned char* data, size_t length, size_t offset) const;
    protected:
        uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t typeFilter, VkMemoryPropertyFlags properties);
    };
}