#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/graphics/GPU.hpp>

namespace RenderEngine
{
    class CommandBuffer
    {
    public:
        CommandBuffer() = delete;
        CommandBuffer(VkCommandPool command_pool, VkQueue queue);
        ~CommandBuffer();
    public:
        const GPU* gpu;
        uint32_t n_commands = 0;
        VkQueue _queue;
        VkCommandPool _command_pool;
        VkCommandBuffer _command_buffer;
    };
}