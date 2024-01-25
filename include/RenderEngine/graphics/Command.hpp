#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/graphics/GPU.hpp>

namespace RenderEngine
{
    class Command
    {
    public:
        Command() = delete;
        Command(VkCommandPool command_pool, VkQueue queue);
        ~Command();
    public:
        const GPU* gpu;
        uint32_t n_commands = 0;
        VkQueue _queue;
        VkCommandPool _command_pool;
        VkCommandBuffer _command_buffer;
    };
}