#pragma once
#include <string>
#include <RenderEngine/graphics/Image.hpp>

namespace RenderEngine
{
    class WindowSettings
    {
    public:
        ///< Title of the window
        std::string title = "window";
        ///< Width of the window
        unsigned int width = 500;
        ///< Height of the window
        unsigned int height = 500;
        ///< If True the window is visible
        bool visible = true;
        ///< If true the window is created in full screen mode
        bool full_screen = false;
        ///< If true, the window is resizable
        bool resizable = true;
        ///< If true the window has borders
        bool borders = true;
        ///< If true the window has transparency (not supported by vulkan drivers on most GPU, excepted for some AMD integrated GPU)
        bool transparent = false;
        ///< If true, the vsync of the window is enabled
        bool vsync = true;
        ///< Number of samples for the Multi Sample Anti Aliasing
        Image::AntiAliasing sample_count = Image::AntiAliasing::X1;
        ///< If true, the swap chain is created at window construction time, otherwise it is only initialized at next 'update' call.
        bool initialize_swapchain = true;
    };
}