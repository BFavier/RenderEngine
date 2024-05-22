#include <RenderEngine/graphics/Buffer.hpp>
using namespace RenderEngine;


Buffer::Buffer(const std::shared_ptr<GPU>& _gpu, size_t bytes_size, VkBufferUsageFlags usage) : gpu(_gpu), _bytes_size(bytes_size)
{
    _allocate_buffer(usage);
    _allocate_memory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

Buffer::~Buffer()
{
    vkDeviceWaitIdle(gpu->_logical_device);
    // delete memory
    vkUnmapMemory(gpu->_logical_device, _vk_memory);
    vkFreeMemory(gpu->_logical_device, _vk_memory, nullptr);
    // delete buffer
    vkDestroyBuffer(gpu->_logical_device, _vk_buffer, nullptr);
}

size_t Buffer::bytes_size() const
{
    return _bytes_size;
}

void Buffer::upload(const uint8_t* data, std::size_t bytes_size, std::size_t offset) const
{
    memcpy(_data+offset, data, bytes_size);
    if (!(_memory_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
    {
        VkMappedMemoryRange  mem_range{};
        mem_range.memory = _vk_memory;
        mem_range.offset = offset;
        mem_range.size = bytes_size;
        vkFlushMappedMemoryRanges(gpu->_logical_device, 1, &mem_range);
    }
}

void Buffer::download(uint8_t* data, std::size_t bytes_size, std::size_t offset) const
{
    memcpy(data, _data+offset, bytes_size);
    if (!(_memory_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
    {
        VkMappedMemoryRange  mem_range{};
        mem_range.memory = _vk_memory;
        mem_range.offset = offset;
        mem_range.size = bytes_size;
        vkInvalidateMappedMemoryRanges(gpu->_logical_device, 1, &mem_range);
    }
}

uint32_t Buffer::_find_memory_type(VkPhysicalDevice physical_device, uint32_t typeFilter, VkMemoryPropertyFlags memory_properties)
{
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);
    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & memory_properties) == memory_properties)
        {
            _memory_properties = mem_properties.memoryTypes[i].propertyFlags;
            return i;
        }
    }
    THROW_ERROR("Failed to find suitable memory type.");
}

void Buffer::_allocate_buffer(VkBufferUsageFlags usage)
{
    const std::shared_ptr<GPU>& _gpu = gpu;
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = _bytes_size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(gpu->_logical_device, &bufferInfo, nullptr, &_vk_buffer) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create vertex buffer!");
    }
}

void Buffer::_allocate_memory(VkMemoryPropertyFlags memory_properties)
{
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(gpu->_logical_device, _vk_buffer, &memRequirements);
    std::shared_ptr<GPU> _gpu = gpu;
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = _find_memory_type(gpu->_physical_device, memRequirements.memoryTypeBits, memory_properties);
    if (vkAllocateMemory(gpu->_logical_device, &allocInfo, nullptr, &_vk_memory) != VK_SUCCESS)
    {
        THROW_ERROR("failed to allocate buffer memory!");
    }
    vkBindBufferMemory(gpu->_logical_device, _vk_buffer, _vk_memory, 0);
    vkMapMemory(gpu->_logical_device, _vk_memory, 0, _bytes_size, 0, reinterpret_cast<void**>(&_data));
}