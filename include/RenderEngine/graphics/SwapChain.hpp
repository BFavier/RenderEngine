#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/graphics/Canvas.hpp>
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
    public: // this object is non copyable
        SwapChain() = delete;
        SwapChain(const SwapChain& other) = delete;
        const SwapChain& operator=(const SwapChain& other) = delete;
    public:
        SwapChain(const std::shared_ptr<GPU>& gpu, const Window& window);
        ~SwapChain();
    public:
        std::shared_ptr<GPU> gpu;
        std::vector<Canvas> canvas;
        VkSwapchainKHR _vk_swap_chain;
    };
}