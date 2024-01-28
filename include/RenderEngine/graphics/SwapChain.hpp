#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/graphics/Image.hpp>
#include <vector>
#include <algorithm>
#include <memory>

namespace RenderEngine
{
    // A swap chain is a Vulkan mechanism to swap images rendered to screen
    class Window;
    class GPU;

    class SwapChain
    {
    public:
        SwapChain() = delete;
        SwapChain(const GPU& gpu, const Window& window);
        ~SwapChain();
    public:
        const GPU& gpu;
        std::vector<Image> images;
        VkSwapchainKHR _swap_chain;
    };
}