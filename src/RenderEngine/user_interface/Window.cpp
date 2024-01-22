#include <RenderEngine/user_interface/Window.hpp>
#include <RenderEngine/user_interface/Keyboard.hpp>
#include <RenderEngine/user_interface/Mouse.hpp>
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/graphics/GPU.hpp>
using namespace RenderEngine;

Window::Window(const GPU& _gpu) : _state(new Handles()), keyboard(*this), mouse(*this), gpu(_gpu), swap_chain(_gpu, *this)
{
}

Window::Window(const Window& other) : _state(other._state), keyboard(other.keyboard), mouse(other.mouse), gpu(other.gpu), swap_chain(other.gpu, *this)
{
}

Window::Window(const GPU& _gpu, const std::string& title, unsigned int width, unsigned int height) : _state(new Handles(title, width, height)), keyboard(*this), mouse(*this), gpu(_gpu), swap_chain(_gpu, *this)
{
}

Window::Window(const GPU& _gpu, const WindowSettings& settings) : _state(new Handles(settings)), keyboard(*this), mouse(*this), gpu(_gpu), swap_chain(_gpu, *this)
{
}

Window::~Window()
{
}

void Window::update()
{
    //Polling events
    _state->_set_unchanged();
    glfwPollEvents();
    //Drawing to screen

    //Engine::shader_test->draw();
    glfwSwapBuffers(_state->_glfw_window);
}

unsigned int Window::x() const
{
    int x;
    glfwGetWindowPos(_state->_glfw_window, &x, nullptr);
    return x;
}

unsigned int Window::y() const
{
    int y;
    glfwGetWindowPos(_state->_glfw_window, nullptr, &y);
    return y;
}

void Window::move(unsigned int x, unsigned int y)
{
    glfwSetWindowPos(_state->_glfw_window, x, y);
}

unsigned int Window::screen_width() const
{
    GLFWmonitor* monitor = glfwGetWindowMonitor(_state->_glfw_window);
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    return mode->width;
}

unsigned int Window::screen_height() const
{
    GLFWmonitor* monitor = glfwGetWindowMonitor(_state->_glfw_window);
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    return mode->height;
}

unsigned int Window::width() const
{
    int w;
    glfwGetWindowSize(_state->_glfw_window, &w, nullptr);
    return w;
}

unsigned int Window::height() const
{
    int h;
    glfwGetWindowSize(_state->_glfw_window, nullptr, &h);
    return h;
}

void Window::resize(unsigned int width, unsigned int height)
{
    glfwSetWindowSize(_state->_glfw_window, width, height);
}

bool Window::full_screen() const
{
    return _state->_window_full_screen;
}

void Window::full_screen(bool enabled)
{
    if (enabled == _state->_window_full_screen)
    {
        return;
    }
    _state->_window_full_screen = enabled;
    if (enabled)
    {
        GLFWmonitor* monitor = glfwGetWindowMonitor(_state->_glfw_window);
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(_state->_glfw_window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
    }
    else
    {
        int x, y, w, h;
        glfwGetWindowPos(_state->_glfw_window, &x, &y);
        glfwGetWindowSize(_state->_glfw_window, &w, &h);
        glfwSetWindowMonitor(_state->_glfw_window, nullptr,  x, y, w, h, GLFW_DONT_CARE);
    }
}

void Window::close()
{
    glfwSetWindowShouldClose(_state->_glfw_window, true);
}

bool Window::closing() const
{
    return glfwWindowShouldClose(_state->_glfw_window);
}

void Window::title(const std::string& name)
{
    _state->_window_title = name;
    return glfwSetWindowTitle(_state->_glfw_window, name.c_str());
}

const std::string& Window::title() const
{
    return _state->_window_title;
}

bool Window::has_focus()
{
    return glfwGetWindowAttrib(_state->_glfw_window, GLFW_FOCUSED);
}

void Window::focus()
{
    glfwFocusWindow(_state->_glfw_window);
}

bool Window::resizable()
{
    return glfwGetWindowAttrib(_state->_glfw_window, GLFW_RESIZABLE);
}

void Window::resizable(bool resizable)
{
    glfwSetWindowAttrib(_state->_glfw_window, GLFW_RESIZABLE, resizable);
}

bool Window::borders()
{
    return glfwGetWindowAttrib(_state->_glfw_window, GLFW_DECORATED);
}

void Window::borders(bool borders)
{
    glfwSetWindowAttrib(_state->_glfw_window, GLFW_DECORATED, borders);
}

bool Window::transparent()
{
    return glfwGetWindowAttrib(_state->_glfw_window, GLFW_TRANSPARENT_FRAMEBUFFER);
}

void Window::transparent(bool transparent)
{
    glfwSetWindowAttrib(_state->_glfw_window, GLFW_TRANSPARENT_FRAMEBUFFER, transparent);
}

bool Window::vsync() const
{
    return _state->_window_vsync;
}

void Window::vsync(bool enabled)
{
    _state->_window_vsync = enabled;
    if (enabled)
    {
        glfwSwapInterval(1);
    }
    else
    {
        glfwSwapInterval(0);
    }
}

const std::shared_ptr<Handles>& Window::_get_state() const
{
    return _state;
}

const VkSurfaceKHR& Window::_get_vk_surface() const
{
    return _state->_vk_surface;
}