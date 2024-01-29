#include <RenderEngine/graphics/Buffer.hpp>
using namespace RenderEngine;


Buffer::Buffer(const GPU& _gpu, VkDeviceSize _size) : gpu(_gpu), size(_size)
{
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkBufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = size;
    info.usage = usage;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(gpu._logical_device, &info, nullptr, &_buffer) != VK_SUCCESS)
    {
        THROW_ERROR("Failed to create buffer.");
    }
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(gpu._logical_device, _buffer, &memRequirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(gpu._physical_device, memRequirements.memoryTypeBits, properties);
    if (vkAllocateMemory(gpu._logical_device, &allocInfo, nullptr, &_buffer_memory) != VK_SUCCESS)
    {
        THROW_ERROR("failed to allocate buffer memory!");
    }
    vkBindBufferMemory(gpu._logical_device, _buffer, _buffer_memory, 0);
}

Buffer::~Buffer()
{
    vkFreeMemory(gpu._logical_device, _buffer_memory, nullptr);
    vkDestroyBuffer(gpu._logical_device, _buffer, nullptr);
}

void Buffer::upload(const unsigned char* data, size_t length, size_t offset)
{
    void* target;
    vkMapMemory(gpu._logical_device, _buffer_memory, static_cast<VkDeviceSize>(offset), length, 0, &target);
    memcpy(target, data, static_cast<size_t>(length));
    vkUnmapMemory(gpu._logical_device, _buffer_memory);
}

void Buffer::download(unsigned char* data, size_t length, size_t offset) const
{
    void* target;
    vkMapMemory(gpu._logical_device, _buffer_memory, static_cast<VkDeviceSize>(offset), length, 0, &target);
    memcpy(target, data, static_cast<size_t>(length));
    vkUnmapMemory(gpu._logical_device, _buffer_memory);
}

uint32_t Buffer::find_memory_type(VkPhysicalDevice physical_device, uint32_t typeFilter, VkMemoryPropertyFlags properties)
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