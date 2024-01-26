#pragma once
#include <map>
#include <string>
#include <cctype> //for function "toupper"
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/user_interface/Button.hpp>
#include <RenderEngine/user_interface/WindowSettings.hpp>

namespace RenderEngine
{
    // WindowState is a class that stores internal state of the window and it's events
    class WindowState
    {
    public:
        WindowState();
        WindowState(const std::string& title, unsigned int width, unsigned int height);
        WindowState(const WindowSettings& settings);
        ~WindowState();
    public:
        unsigned int _window_width = 0;
        unsigned int _window_height = 0;
        std::string _window_title;
        bool _window_full_screen = false;
        bool _window_vsync = false;
        double _mouse_x = 0;
        double _mouse_y = 0;
        double _mouse_dx = 0;
        double _mouse_dy = 0;
        double _mouse_wheel_x = 0;
        double _mouse_wheel_y = 0;
        bool _mouse_hidden = false;
        std::map<const std::string, Button> _mouse_buttons;
        std::map<const std::string, Button> _keyboard_buttons;
        GLFWwindow* _glfw_window;
        VkSurfaceKHR _vk_surface;
    public:
        void _set_unchanged();
    public:
        static void _window_resize_callback(GLFWwindow* window, int width, int height);
        static void _mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
        static void _mouse_position_callback(GLFWwindow* window, double xpos, double ypos);
        static void _mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
        static void _keyboard_button_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static std::string _get_key_name(int key, int scancode);
    };
}