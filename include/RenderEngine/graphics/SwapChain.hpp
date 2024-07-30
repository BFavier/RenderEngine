#pragma once
#include <RenderEngine/utilities/External.hpp>
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
    public:
        void present_frame(); // Queue the frame for presentation to screen.
        Canvas& get_frame(); // Returns the frame to render to.
    protected:
        int _frame_index_next = -1;
        std::queue<std::pair<VkSemaphore, int>> _frame_available_semaphores; // a queue of (semaphore, previous frame index) pairs. The semaphore is used to wait the acquisition of a frame. The frame index is the index of the previous frame that was acquired using the semaphore.
        VkSwapchainKHR _vk_swap_chain;
    };
}