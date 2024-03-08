#include <RenderEngine/graphics/Buffer.hpp>
using namespace RenderEngine;


Buffer::Buffer(const std::shared_ptr<GPU>& _gpu, size_t _size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) : gpu(_gpu), size(_size)
{
    _allocate_buffer(usage);
    _allocate_memory(properties);
}

Buffer::~Buffer()
{
}

void Buffer::upload(const void* data, size_t length, size_t offset)
{
    void* target;
    vkMapMemory(gpu->_logical_device, *_vk_memory, static_cast<VkDeviceSize>(offset), length, 0, &target);
    memcpy(target, data, length);
    vkUnmapMemory(gpu->_logical_device, *_vk_memory);
}

void Buffer::download(void* data, size_t length, size_t offset) const
{
    void* target;
    vkMapMemory(gpu->_logical_device, *_vk_memory, static_cast<VkDeviceSize>(offset), length, 0, &target);
    memcpy(data, target, length);
    vkUnmapMemory(gpu->_logical_device, *_vk_memory);
}

uint32_t Buffer::_find_memory_type(VkPhysicalDevice physical_device, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);
    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    THROW_ERROR("Failed to find suitable memory type.");
}

void Buffer::_allocate_buffer(VkBufferUsageFlags usage)
{
    const std::shared_ptr<GPU>& _gpu = gpu;
    _vk_buffer.reset(new VkBuffer, [_gpu](VkBuffer* vk_buffer){vkDestroyBuffer(_gpu->_logical_device, *vk_buffer, nullptr);vk_buffer = nullptr;});
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(gpu->_logical_device, &bufferInfo, nullptr, _vk_buffer.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create vertex buffer!");
    }
}

void Buffer::_allocate_memory(VkMemoryPropertyFlags properties)
{
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(gpu->_logical_device, *_vk_buffer, &memRequirements);
    const std::shared_ptr<GPU>& _gpu = gpu;
    _vk_memory.reset(new VkDeviceMemory, [_gpu](VkDeviceMemory* memory){vkFreeMemory(_gpu->_logical_device, *memory, nullptr);*memory=nullptr;});
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = _find_memory_type(gpu->_physical_device, memRequirements.memoryTypeBits, properties);
    if (vkAllocateMemory(gpu->_logical_device, &allocInfo, nullptr, _vk_memory.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to allocate buffer memory!");
    }
}