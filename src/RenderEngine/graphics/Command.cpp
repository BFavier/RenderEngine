#include <RenderEngine/graphics/Command.hpp>
using namespace RenderEngine;


Command::Command(const GPU& _gpu, VkCommandPool command_pool, VkQueue queue) : gpu(_gpu), _command_pool(command_pool), _queue(queue)
{
    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandPool = command_pool;
    info.commandBufferCount = 1;
    vkAllocateCommandBuffers(gpu._logical_device, &info, &_command_buffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(_command_buffer, &beginInfo);
}

Command::~Command()
{
    vkEndCommandBuffer(_command_buffer);
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = n_commands;
    submitInfo.pCommandBuffers = &_command_buffer;
    vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_queue);
    vkFreeCommandBuffers(gpu._logical_device, _command_pool, 1, &_command_buffer);
}