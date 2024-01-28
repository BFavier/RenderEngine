#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/graphics/GPU.hpp>

namespace RenderEngine
{
    class Pipeline
    {
    public:
        Pipeline() = delete;
        Pipeline(const GPU& gpu);
        ~Pipeline();
    public:
        VkPipelineLayout _vk_graphic_pipeline = VK_NULL_HANDLE;
    };
}