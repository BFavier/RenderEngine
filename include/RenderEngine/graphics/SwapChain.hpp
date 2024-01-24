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
        void operator=(const SwapChain& other);
    public:
        const GPU* gpu;
        std::shared_ptr<std::vector<Image>> images;
        std::shared_ptr<VkSwapchainKHR> _swap_chain;
    public:
        static void _deallocate_swap_chain(VkSwapchainKHR* swap_chain, const GPU& gpu); 
    };
}