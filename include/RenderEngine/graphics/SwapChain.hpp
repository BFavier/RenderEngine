#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/graphics/Canvas.hpp>
#include <vector>
#include <queue>
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
        std::vector<Canvas*> frames;
        std::queue<VkSemaphore> frame_available_semaphores;
        VkSwapchainKHR _vk_swap_chain;
    public:
        void present_next_frame(); // present the next frame to screen
        Canvas* get_current_frame(); // Return the current frame.
        Canvas& get_next_frame(); // Return the next frame. Acquire it if it wasn't already.
    protected:
        int _frame_index_current = -1;
        int _frame_index_next = -1;
    };
}