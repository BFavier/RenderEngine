#pragma once
#include <string>

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
        ///< If true the window is transparent (if the alpha channel of the displayed image is inferior to 1.)
        bool transparent = false;
        ///< If true, the vsync of the window is enabled
        bool vsync = true;
        ///< Number of samples for the Multi Sample Anti Aliasing
        unsigned int anti_aliasing = 1;
    };
}