#include <RenderEngine/graphics/Buffer.hpp>
using namespace RenderEngine;


Buffer::Buffer(const std::shared_ptr<GPU>& _gpu, size_t bytes_size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties) : gpu(_gpu), _bytes_size(bytes_size)
{
    _allocate_buffer(usage);
    _allocate_memory(memory_properties);
}

Buffer::~Buffer()
{
}

size_t Buffer::bytes_size() const
{
    return _bytes_size;
}

void Buffer::upload(const void* data) const
{
    memcpy(_data, data, _bytes_size);
    if (!(_memory_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
    {
        VkMappedMemoryRange  mem_range{};
        mem_range.memory = *_vk_memory;
        mem_range.offset = 0;
        mem_range.size = _bytes_size;
        vkFlushMappedMemoryRanges(gpu->_logical_device, 1, &mem_range);
    }
}

void Buffer::download(void* data) const
{
    memcpy(data, _data, _bytes_size);
    if (!(_memory_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
    {
        VkMappedMemoryRange  mem_range{};
        mem_range.memory = *_vk_memory;
        mem_range.offset = 0;
        mem_range.size = _bytes_size;
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
    _vk_buffer.reset(new VkBuffer, [_gpu](VkBuffer* vk_buffer){vkDeviceWaitIdle(_gpu->_logical_device);vkDestroyBuffer(_gpu->_logical_device, *vk_buffer, nullptr);vk_buffer = nullptr;});
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = _bytes_size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(gpu->_logical_device, &bufferInfo, nullptr, _vk_buffer.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create vertex buffer!");
    }
}

void Buffer::_allocate_memory(VkMemoryPropertyFlags memory_properties)
{
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(gpu->_logical_device, *_vk_buffer, &memRequirements);
    const std::shared_ptr<GPU>& _gpu = gpu;
    const std::shared_ptr <VkDeviceMemory>& vk_memory = _vk_memory;
    VkDeviceMemory* memory = new VkDeviceMemory();
    _vk_memory.reset(memory, [&_gpu, &memory](VkDeviceMemory* memory) {vkDeviceWaitIdle(_gpu->_logical_device); vkUnmapMemory(_gpu->_logical_device, *memory); vkFreeMemory(_gpu->_logical_device, *memory, nullptr);});
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = _find_memory_type(gpu->_physical_device, memRequirements.memoryTypeBits, memory_properties);
    if (vkAllocateMemory(gpu->_logical_device, &allocInfo, nullptr, _vk_memory.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to allocate buffer memory!");
    }
    vkBindBufferMemory(gpu->_logical_device, *_vk_buffer, *_vk_memory, 0);
    vkMapMemory(gpu->_logical_device, *_vk_memory, 0, _bytes_size, 0, &_data);
}