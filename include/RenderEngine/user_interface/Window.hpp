#pragma once
#include <string>
#include <RenderEngine/utilities/External.hpp>
#include "WindowSettings.hpp"
#include "WindowState.hpp"
#include "Keyboard.hpp"
#include "Mouse.hpp"
#include <RenderEngine/graphics/SwapChain.hpp>

namespace RenderEngine
{

    class Window
    {
    public:
        Window() = delete;
        Window(const GPU& gpu);
        Window(const Window& other);
        Window(const GPU& gpu, const std::string& title, unsigned int width, unsigned int height);
        Window(const GPU& gpu, const WindowSettings& settings);
        ~Window();
    public:
        void operator=(const Window& other);
    public:
        ///< Update the window's display, and the window's inputs (keyboard and mouse)
        void update();
        ///< Get the x/y position of the window
        unsigned int x() const;
        unsigned int y() const;
        ///< Move the window to the given position on it's screen
        void move(unsigned int x, unsigned int y);
        ///< Get the width/height of the screen the window is on
        unsigned int screen_width() const;
        unsigned int screen_height() const;
        ///< Get the widht/height of the window
        unsigned int width() const;
        unsigned int height() const;
        ///< Resize the window to the given width and height
        void resize(unsigned int width, unsigned int height);
        ///< Returns true if the window is in fullscreen
        bool full_screen() const;
        ///< Set whether the window should be in full screen
        void full_screen(bool enabled);
        ///< Close the window
        void close();
        ///< Returns true if the user asked for the window to close
        bool closing() const;
        ///< Set the title
        void title(const std::string& name);
        ///< Returns the title of the window
        const std::string& title() const;
        ///< Returns true if the window is currently selected
        bool has_focus();
        ///< Make this window the currently selected window
        void focus();
        ///< Returns true if the window is resizable by the user
        bool resizable();
        ///< Make the window resizable by the user or not
        void resizable(bool resizable);
        ///< Returns true if the window has borders
        bool borders();
        ///< Enable or disable the borders of the window
        void borders(bool borders);
        ///< Returns true if the window is transparent (it can been see through when the alpha of the image displayed is inferior to 1.)
        bool transparent();
        ///< Set whether the window is transparent or not
        void transparent(bool transparent);
        ///< Returns true if the vertical syncing is enabled (limits the FPS to monitor's refresh rate)
        bool vsync() const;
        ///< Enables or disable vertical syncing
        void vsync(bool enabled);
    public:
        const std::shared_ptr<WindowState>& _get_state() const;
        const VkSurfaceKHR& _get_vk_surface() const;
    public:
        std::shared_ptr<WindowState> _state; // This must be above keyboard and mouse in the class definition
        Keyboard keyboard;
        Mouse mouse;
        const GPU* gpu;
        SwapChain swap_chain;
    };
}
