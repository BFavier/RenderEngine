#pragma once
#include <string>
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/user_interface/WindowSettings.hpp>
#include <RenderEngine/user_interface/Keyboard.hpp>
#include <RenderEngine/user_interface/Mouse.hpp>
#include <RenderEngine/graphics/SwapChain.hpp>

namespace RenderEngine
{

    class Window
    {

    friend class Mouse;  // Mouse can access the _glfw_window attribute
    friend class SwapChain; // SwapChain can access the _vk_surface and _glfw_window attributes
    friend class GPU; // GPU can access the _vk_surface attribute
    friend class Internal; // RenderEngine::Internal can create dummy windows
    
    public: // This class is non copyable
        Window() = delete;
        Window(const Window& other) = delete;
        Window& operator=(const Window& other) = delete;
        Window(Window&&) = default;
        Window& operator=(Window&&) = default;
    public:
        Window(const std::shared_ptr<GPU>& gpu, const std::string& title, unsigned int width, unsigned int height);
        Window(const std::shared_ptr<GPU>& gpu, const WindowSettings& settings);
        ~Window();
    protected: // This constructor allows to create a dummy window without GPU (swapchain creation will fail)
        Window(const WindowSettings& settings);
    public:
        std::shared_ptr<GPU> gpu;
        Keyboard keyboard;
        Mouse mouse;
    public:
        ///< returns the next Canvas to draw to from the swapchain. Wait until an image is available
        Canvas& back_frame();
        ///< Update the window's display, and the window's inputs states (keyboard and mouse)
        void update();
        ///< Get the x position of the window in the screen
        int x() const;
        ///< Get the y position of the window in the screen
        int y() const;
        ///< Move the window to the given position on it's screen
        void move(int x, int y);
        ///< Get the width of the screen the window is on
        unsigned int screen_width() const;
        ///< Get the height of the screen the window is on
        unsigned int screen_height() const;
        ///< Get the widht of the window
        unsigned int width() const;
        ///< Get the height of the window
        unsigned int height() const;
        ///< Resize the window to the given width and height
        void resize(unsigned int width, unsigned int height);
        ///< Returns true if the window is in fullscreen
        bool full_screen() const;
        ///< Set whether the window should be in full screen
        void full_screen(bool enabled);
        ///< Request the window to close
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
        static void _window_resize_callback(GLFWwindow* window, int width, int height);
        static void _mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
        static void _mouse_position_callback(GLFWwindow* window, double xpos, double ypos);
        static void _mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
        static void _keyboard_button_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    protected:
        void _initialize(const WindowSettings& settings);
        void _recreate_swapchain();
        void _delete_swapchain();
        void _set_unchanged();
    protected:
        int _x;
        int _y;
        unsigned int _window_width = 0;
        unsigned int _window_height = 0;
        std::string _window_title;
        bool _window_full_screen = false;
        bool _window_vsync = false;
        SwapChain* _swap_chain = nullptr;
        GLFWwindow* _glfw_window = nullptr;
        VkSurfaceKHR _vk_surface = VK_NULL_HANDLE;
    };
}
