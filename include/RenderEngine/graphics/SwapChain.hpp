#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <vector>
#include <algorithm>
#include <memory>

namespace RenderEngine
{
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
        VkSwapchainKHR _swap_chain;
        std::vector<VkImage> _vk_images;
        VkFormat _image_format;
    protected:
        VkSurfaceFormatKHR _choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
        VkPresentModeKHR _choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
        VkExtent2D _choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, const Window& window);
    };
}