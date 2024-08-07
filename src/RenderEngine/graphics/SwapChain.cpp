#include <RenderEngine/graphics/SwapChain.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <RenderEngine/user_interface/Window.hpp>
#include <RenderEngine/utilities/Macro.hpp>
using namespace RenderEngine;

SwapChain::SwapChain(const GPU* _gpu, const Window& window) : gpu(_gpu)
{
    if (!gpu->_graphics_queue.has_value() || !gpu->_present_queue.has_value())
    {
        THROW_ERROR("The provided GPU does not support presenting to windows");
    }
    // Query window surface capacibilties
    const VkSurfaceKHR& surface = window._vk_surface;
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu->_physical_device, surface, &capabilities);
    // Choosing surface format
    uint32_t n_supported_formats;
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu->_physical_device, surface, &n_supported_formats, nullptr);
    if (n_supported_formats == 0)
    {
        THROW_ERROR("The GPU supports 0 surface formats.");
    }
    std::vector<VkSurfaceFormatKHR> supported_formats(n_supported_formats);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu->_physical_device, surface, &n_supported_formats, supported_formats.data());
    VkSurfaceFormatKHR surface_format;
    bool format_found = false;
    for (const VkSurfaceFormatKHR& sf : supported_formats)
    {
        if (sf.format == static_cast<VkFormat>(ImageFormat::RGBA) && sf.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            surface_format = sf;
            format_found = true;
            break;
        }
    }
    if (!format_found)
    {
        THROW_ERROR("GPU does not support an appropriate surface format.");
    }
    // Choosing present mode
    uint32_t n_present_modes;
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu->_physical_device, surface, &n_present_modes, nullptr);
    if (n_present_modes == 0)
    {
        THROW_ERROR("The GPU supports 0 presentation modes to a screen.");
    }
    std::vector<VkPresentModeKHR> supported_present_modes(n_present_modes);
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu->_physical_device, surface, &n_present_modes, supported_present_modes.data());
    VkPresentModeKHR present_mode;
    if (!window.vsync_is_enabled() &&
        (std::find(supported_present_modes.begin(), supported_present_modes.end(), VK_PRESENT_MODE_IMMEDIATE_KHR) != supported_present_modes.end()))
    {
        present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
    else if (window.vsync_is_enabled() &&
        (std::find(supported_present_modes.begin(), supported_present_modes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != supported_present_modes.end()))
    {
        present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
    }
    else if (std::find(supported_present_modes.begin(), supported_present_modes.end(), VK_PRESENT_MODE_FIFO_KHR) != supported_present_modes.end())
    {
        present_mode = VK_PRESENT_MODE_FIFO_KHR;
    }
    else
    {
        present_mode = supported_present_modes.front();
    }
    // Choosing extent
    VkExtent2D extent;
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        extent = capabilities.currentExtent;
    }
    else // for Mac retina screens ... 
    {
        int width, height;
        glfwGetFramebufferSize(window._glfw_window, &width, &height);
        extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }
    // check supported window alpha transparency
    std::vector<VkCompositeAlphaFlagBitsKHR> candidates_alpha_compose = {VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR};
    bool found_compatible = false;
    VkCompositeAlphaFlagBitsKHR alpha_compose;
    for (const VkCompositeAlphaFlagBitsKHR& compose : candidates_alpha_compose)
    {
        if ((capabilities.supportedCompositeAlpha & compose) == compose)
        {
            alpha_compose = compose;
            found_compatible = true;
        }
    }
    if (!found_compatible)
    {
        THROW_ERROR("Failed to find a supported VkCompositeAlphaFlagBitsKHR for window surface.")
    }
    // create swap chain
    uint32_t image_count = std::min(capabilities.minImageCount + 1, capabilities.maxImageCount);
    VkSwapchainCreateInfoKHR swap_chain_infos{};
    swap_chain_infos.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap_chain_infos.surface = surface;
    swap_chain_infos.minImageCount = image_count;
    swap_chain_infos.imageFormat = surface_format.format;
    swap_chain_infos.imageColorSpace = surface_format.colorSpace;
    swap_chain_infos.imageExtent = extent;
    swap_chain_infos.imageArrayLayers = 1;
    swap_chain_infos.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL | VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    if (gpu->_graphics_queue != gpu->_present_queue)
    {
        std::vector<uint32_t> family_indices = {std::get<0>(gpu->_graphics_queue.value()), std::get<0>(gpu->_present_queue.value())};
        swap_chain_infos.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swap_chain_infos.queueFamilyIndexCount = family_indices.size();
        swap_chain_infos.pQueueFamilyIndices = family_indices.data();
    }
    else
    {
        swap_chain_infos.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swap_chain_infos.queueFamilyIndexCount = 0;
        swap_chain_infos.pQueueFamilyIndices = nullptr;
    }
    swap_chain_infos.preTransform = capabilities.currentTransform;
    swap_chain_infos.compositeAlpha = alpha_compose;
    swap_chain_infos.presentMode = present_mode;
    swap_chain_infos.clipped = VK_TRUE;
    swap_chain_infos.oldSwapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(gpu->_logical_device, &swap_chain_infos, nullptr, &_vk_swap_chain) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create the swap chain");
    }
    // Getting the vkImages
    vkGetSwapchainImagesKHR(gpu->_logical_device, _vk_swap_chain, &image_count, nullptr); // just in case our requested image count was not respected, query it again
    std::vector<VkImage> vk_images(image_count, VK_NULL_HANDLE);
    vkGetSwapchainImagesKHR(gpu->_logical_device, _vk_swap_chain, &image_count, vk_images.data());
    for (int i=0; i<image_count; i++)
    {
        // create the Canvas of the obtained frame
        frames.push_back(new Canvas(gpu, vk_images[i], extent.width, extent.height, window._window_sample_count));
        // create a semaphore
        VkSemaphore semaphore;
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        if (vkCreateSemaphore(gpu->_logical_device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
        {
            THROW_ERROR("failed to create VkSemaphore");
        }
        _frame_available_semaphores.push(std::make_pair(semaphore, -1));
    }
}

SwapChain::~SwapChain()
{
    vkDeviceWaitIdle(gpu->_logical_device);
    while (_frame_available_semaphores.size() > 0)
    {
        vkDestroySemaphore(gpu->_logical_device, _frame_available_semaphores.front().first, nullptr);
        _frame_available_semaphores.pop();
    }
    for (Canvas* canvas : frames)
    {
        delete canvas;
    }
    frames.clear();
    vkDestroySwapchainKHR(gpu->_logical_device, _vk_swap_chain, nullptr);
}


void SwapChain::present_frame()
{
    Canvas& frame = get_frame();
    if (frame.is_recording())
    {
        THROW_ERROR("Tried presenting swapchain frame to screen, but 'render' has not been called.");
    }
    uint32_t i = static_cast<uint32_t>(_frame_index);
    VkResult result = VK_INCOMPLETE;
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = frame.is_rendering() ? 1 : 0;
    presentInfo.pWaitSemaphores = frame.is_rendering() ? &frame._vk_rendered_semaphore : nullptr;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &_vk_swap_chain;
    presentInfo.pImageIndices = &i;
    presentInfo.pResults = &result; // Optional
    vkQueuePresentKHR(std::get<1>(gpu->_present_queue.value()), &presentInfo);
    if (result != VK_SUCCESS)
    {
        THROW_ERROR("Failed to present swapchaoin image to screen with VkResult code : " + std::to_string(result));
    }
    _frame_index = -1; // a new frame must be queried by get_frame
}


Canvas& SwapChain::get_frame()
{
    if (_frame_index < 0)
    {
        uint32_t i = std::numeric_limits<uint32_t>::max();
        int i_waited_frame;
        VkSemaphore semaphore;
        std::tie(semaphore, i_waited_frame) = _frame_available_semaphores.front();
        if (i_waited_frame >= 0)
        {
            frames[i_waited_frame]->wait_completion(); // otherwise we might try to signal a semaphore that was previously signaled, and not yet waited on.
        }
        vkAcquireNextImageKHR(gpu->_logical_device, _vk_swap_chain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &i);
        frames[i]->_wait_semaphores.insert(semaphore);
        _frame_available_semaphores.pop();
        _frame_available_semaphores.push(std::make_pair(semaphore, static_cast<int>(i)));
        _frame_index = static_cast<int>(i);
    }
    return *frames[_frame_index];
}