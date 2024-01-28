#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/graphics/GPU.hpp>

namespace RenderEngine
{
    class Command
    {
    public:
        Command() = delete;
        Command(const GPU& gpu, VkCommandPool command_pool, VkQueue queue);
        ~Command();
    public:
        const GPU& gpu;
        uint32_t n_commands = 0;
        VkQueue _queue = VK_NULL_HANDLE;
        VkCommandPool _command_pool = VK_NULL_HANDLE;
        VkCommandBuffer _command_buffer = VK_NULL_HANDLE;
    };
}