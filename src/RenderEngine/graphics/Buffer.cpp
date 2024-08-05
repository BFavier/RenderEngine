#include <RenderEngine/graphics/Buffer.hpp>
#include <RenderEngine/utilities/Macro.hpp>
using namespace RenderEngine;


Buffer::Buffer(const GPU* _gpu, size_t bytes_size, VkBufferUsageFlags usage) : gpu(_gpu), _bytes_size(bytes_size)
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

void Buffer::upload(const void* data, std::size_t bytes_size, std::size_t offset) const
{
    memcpy(reinterpret_cast<uint8_t*>(_data)+offset, data, bytes_size);
    if (!(_memory_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
    {
        VkMappedMemoryRange  mem_range{};
        mem_range.memory = _vk_memory;
        mem_range.offset = offset;
        mem_range.size = bytes_size;
        vkFlushMappedMemoryRanges(gpu->_logical_device, 1, &mem_range);
    }
}

void Buffer::download(void* data, std::size_t bytes_size, std::size_t offset) const
{
    memcpy(data, reinterpret_cast<uint8_t*>(_data)+offset, bytes_size);
    if (!(_memory_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
    {
        VkMappedMemoryRange  mem_range{};
        mem_range.memory = _vk_memory;
        mem_range.offset = offset;
        mem_range.size = bytes_size;
        vkInvalidateMappedMemoryRanges(gpu->_logical_device, 1, &mem_range);
    }
}

void Buffer::_allocate_buffer(VkBufferUsageFlags usage)
{
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
    // finding memory type
    uint32_t memoryTypeIndex = std::numeric_limits<uint32_t>::max();
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(gpu->_physical_device, &mem_properties);
    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
    {
        if ((memRequirements.memoryTypeBits & (1 << i))
            &&
            ((mem_properties.memoryTypes[i].propertyFlags & memory_properties) == memory_properties))
        {
            memoryTypeIndex = i;
            _memory_properties = mem_properties.memoryTypes[i].propertyFlags;
            break;
        }
    }
    if (memoryTypeIndex == std::numeric_limits<uint32_t>::max())
    {
        THROW_ERROR("Failed to find suitable memory type.");
    }
    // allocate memory
    std::size_t required_memory_bytes = ((memRequirements.size / memRequirements.alignment) + (memRequirements.size % memRequirements.alignment == 0 ? 0 : 1)) * memRequirements.alignment;
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = required_memory_bytes;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    if (vkAllocateMemory(gpu->_logical_device, &allocInfo, nullptr, &_vk_memory) != VK_SUCCESS)
    {
        THROW_ERROR("failed to allocate buffer memory!");
    }
    // bind and map
    vkBindBufferMemory(gpu->_logical_device, _vk_buffer, _vk_memory, 0);
    vkMapMemory(gpu->_logical_device, _vk_memory, 0, _bytes_size, 0, reinterpret_cast<void**>(&_data));
}