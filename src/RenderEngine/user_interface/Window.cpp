#include <RenderEngine/Internal.hpp>
#include <RenderEngine/user_interface/Window.hpp>
#include <RenderEngine/user_interface/Keyboard.hpp>
#include <RenderEngine/user_interface/Mouse.hpp>
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/graphics/GPU.hpp>
using namespace RenderEngine;

Window::Window(const WindowSettings& settings) : keyboard(*this), mouse(*this)
{
    _initialize(settings);
}

Window::Window(const std::shared_ptr<GPU>& _gpu, const std::string& title, unsigned int width, unsigned int height) : gpu(_gpu), keyboard(*this), mouse(*this)
{
    WindowSettings settings;
    settings.title = title;
    settings.width = width;
    settings.height = height;
    _initialize(settings);
}

Window::Window(const std::shared_ptr<GPU>& _gpu, const WindowSettings& settings) : gpu(_gpu), keyboard(*this), mouse(*this)
{
    _initialize(settings);
}

Window::~Window()
{
    delete _swap_chain;
    vkDestroySurfaceKHR(Internal::get_vulkan_instance(), _vk_surface, nullptr);
    glfwDestroyWindow(_glfw_window);
}


Canvas* Window::current_frame()
{
    return _swap_chain->get_current_frame();
}


Canvas* Window::next_frame()
{
    if (_swap_chain == nullptr)
    {
        return nullptr;
    }
    return &_swap_chain->get_next_frame();
}


void Window::update()
{
    // get the back frame and render if not done already 
    Canvas* frame = next_frame();
    if (frame != nullptr)
    {
        frame->render();
        _swap_chain->present_next_frame();
    }
    // polling events (might delete swap chain by window resizing callback function)
    _set_unchanged();
    glfwPollEvents();
    // recreating swapchain if needed
    if (_swap_chain == nullptr)
    {
        _recreate_swapchain();
    }
}


void Window::_initialize(const WindowSettings& settings)
{
    //Create the GLFW window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, settings.visible);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, settings.transparent);
    glfwWindowHint(GLFW_DECORATED, settings.borders);
    unsigned int width = settings.width;
    unsigned int height = settings.height;
    GLFWmonitor* monitor = nullptr;
    const GLFWvidmode* mode = nullptr;
    _window_full_screen = settings.full_screen;
    if (settings.full_screen)
    {
        monitor = glfwGetPrimaryMonitor();
        mode = glfwGetVideoMode(monitor);
        width = mode->width;
        height = mode->height;
        glfwSetWindowSize(_glfw_window, width, height);
    }
    _window_vsync = settings.vsync;
    _window_sample_count = settings.sample_count;
    _window_title = settings.title;
    _glfw_window = glfwCreateWindow(width, height, _window_title.c_str(), monitor, nullptr);
    if (_glfw_window == nullptr)
    {
        THROW_ERROR("Failed to create the window");
    }
    _window_width = width;
    _window_height = height;
    //Link to the keyboard and mouse events
    glfwSetWindowUserPointer(_glfw_window, this);
    // Setup mouse events
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(_glfw_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    glfwSetMouseButtonCallback(_glfw_window, _mouse_button_callback);
    glfwSetCursorPosCallback(_glfw_window, _mouse_position_callback);
    glfwSetScrollCallback(_glfw_window, _mouse_scroll_callback);
    double xpos, ypos;
    glfwGetCursorPos(_glfw_window, &xpos, &ypos);
    mouse._x = static_cast<unsigned int>(xpos);
    mouse._y = static_cast<unsigned int>(xpos);
    mouse._buttons["LEFT CLICK"];
    mouse._buttons["RIGHT CLICK"];
    mouse._buttons["MIDDLE CLICK"];
    mouse._buttons["MB4"];
    mouse._buttons["MB5"];
    mouse._buttons["MB6"];
    mouse._buttons["MB7"];
    mouse._buttons["MB8"];
    // Setup keyboard events
    glfwSetKeyCallback(_glfw_window, _keyboard_button_callback);
    for (int i=0; i<357; i++)
    {
        std::string name = keyboard._get_key_name(i, 0);
        keyboard._set_key(name, Button());
    }
    // Setup window events
    glfwSetWindowSizeCallback(_glfw_window, _window_resize_callback);
    // Create the vkSurface
    VkResult result = glfwCreateWindowSurface(Internal::get_vulkan_instance(), _glfw_window, NULL, &_vk_surface);
    if (result != VK_SUCCESS)
    {
        THROW_ERROR("Failed to create the Vulkan surface");
    }
    // create the swap chain
    if (settings.initialize_swapchain)
    {
        _recreate_swapchain();
    }
}

void Window::_recreate_swapchain()
{
    if (_swap_chain != nullptr)
    {
        _delete_swapchain();
    }
    if (_window_width > 0 && _window_height > 0)
    {
        _swap_chain = new SwapChain(gpu, *this);
    }
}

void Window::_delete_swapchain()
{
    vkDeviceWaitIdle(gpu->_logical_device);
    delete _swap_chain;
    _swap_chain = nullptr;
}

void Window::move(int x, int y)
{
    glfwSetWindowPos(_glfw_window, x, y);
}

int Window::x() const
{
    return _x;
}

int Window::y() const
{
    return _y;
}

unsigned int Window::screen_width() const
{
    GLFWmonitor* monitor = glfwGetWindowMonitor(_glfw_window);
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    return static_cast<unsigned int>(mode->width);
}

unsigned int Window::screen_height() const
{
    GLFWmonitor* monitor = glfwGetWindowMonitor(_glfw_window);
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    return static_cast<unsigned int>(mode->height);
}

unsigned int Window::width() const
{
    int w;
    glfwGetWindowSize(_glfw_window, &w, nullptr);
    return static_cast<unsigned int>(w);
}

unsigned int Window::height() const
{
    int h;
    glfwGetWindowSize(_glfw_window, nullptr, &h);
    return static_cast<unsigned int>(h);
}

void Window::resize(unsigned int width, unsigned int height)
{
    glfwSetWindowSize(_glfw_window, static_cast<int>(width), static_cast<int>(height));
    _delete_swapchain();
}

bool Window::full_screen() const
{
    return _window_full_screen;
}

void Window::full_screen(bool enabled)
{
    if (enabled == _window_full_screen)
    {
        return;
    }
    _window_full_screen = enabled;
    if (enabled)
    {
        GLFWmonitor* monitor = glfwGetWindowMonitor(_glfw_window);
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(_glfw_window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
    }
    else
    {
        int x, y, w, h;
        glfwGetWindowPos(_glfw_window, &x, &y);
        glfwGetWindowSize(_glfw_window, &w, &h);
        glfwSetWindowMonitor(_glfw_window, nullptr,  x, y, w, h, GLFW_DONT_CARE);
    }
    _delete_swapchain();
}

void Window::close()
{
    glfwSetWindowShouldClose(_glfw_window, true);
}

bool Window::closing() const
{
    return glfwWindowShouldClose(_glfw_window);
}

void Window::title(const std::string& name)
{
    _window_title = name;
    return glfwSetWindowTitle(_glfw_window, name.c_str());
}

const std::string& Window::title() const
{
    return _window_title;
}

bool Window::has_focus()
{
    return glfwGetWindowAttrib(_glfw_window, GLFW_FOCUSED);
}

void Window::focus()
{
    glfwFocusWindow(_glfw_window);
}

bool Window::resizable()
{
    return glfwGetWindowAttrib(_glfw_window, GLFW_RESIZABLE);
}

void Window::resizable(bool resizable)
{
    glfwSetWindowAttrib(_glfw_window, GLFW_RESIZABLE, resizable);
}

bool Window::borders()
{
    return glfwGetWindowAttrib(_glfw_window, GLFW_DECORATED);
}

void Window::borders(bool borders)
{
    glfwSetWindowAttrib(_glfw_window, GLFW_DECORATED, borders);
}

bool Window::transparent()
{
    return glfwGetWindowAttrib(_glfw_window, GLFW_TRANSPARENT_FRAMEBUFFER);
}

void Window::transparent(bool transparent)
{
    glfwSetWindowAttrib(_glfw_window, GLFW_TRANSPARENT_FRAMEBUFFER, transparent);
}

bool Window::vsync() const
{
    return _window_vsync;
}

void Window::vsync(bool enabled)
{
    if (_window_vsync == enabled)
    {
        return;
    }
    _window_vsync = enabled;
    if (enabled)
    {
        glfwSwapInterval(1);
    }
    else
    {
        glfwSwapInterval(0);
    }
    _delete_swapchain();
}

void Window::_set_unchanged()
{
    for (std::pair<const std::string, Button>& key : keyboard._keys)
    {
        key.second.was_pressed = false;
        key.second.was_released = false;
    }
    for (std::pair<const std::string, Button>& button : mouse._buttons)
    {
        button.second.was_pressed = false;
        button.second.was_released = false;
    }
    mouse._dx = 0.;
    mouse._dy = 0.;
    mouse._wheel_dx = 0.;
    mouse._wheel_dy = 0.;
}

void Window::_window_resize_callback(GLFWwindow* window, int width, int height)
{
    Window* h = static_cast<Window*>(glfwGetWindowUserPointer(window));
    h->_window_width = width;
    h->_window_height = height;
    h->_delete_swapchain();
}

void Window::_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    (void)mods;//Silence the annoying unused parameter warning
    Window* h = static_cast<Window*>(glfwGetWindowUserPointer(window));
    std::string name;
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        name = "LEFT CLICK";
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        name = "RIGHT CLICK";
    }
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        name = "MIDDLE CLICK";
    }
    else
    {
        name = "MB"+std::to_string(button+1);
    }
    Button status;
    if (action == GLFW_PRESS)
    {
        status.down = true;
        status.was_pressed = true;
    }
    else if (action == GLFW_RELEASE)
    {
        status.down = false;
        status.was_released = true;
    }
    h->mouse._set_button(name, status);
}

void Window::_mouse_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    Window* h = static_cast<Window*>(glfwGetWindowUserPointer(window));
    h->mouse._dx = xpos - h->mouse._x;
    h->mouse._dy = ypos - h->mouse._y;
    h->mouse._x = xpos;
    h->mouse._y = ypos;
}

void Window::_mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Window* h = static_cast<Window*>(glfwGetWindowUserPointer(window));
    h->mouse._wheel_dx = xoffset;
    h->mouse._wheel_dy = yoffset;
}

void Window::_keyboard_button_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)mods;//Silence the annoying unused parameter warning
    Window* h = static_cast<Window*>(glfwGetWindowUserPointer(window));
    std::string name = h->keyboard._get_key_name(key, scancode);
    Button button;
    if (action == GLFW_PRESS)
    {
        button.down = true;
        button.was_pressed = true;
    }
    else if (action == GLFW_RELEASE)
    {
        button.down = false;
        button.was_released = true;
    }
    h->keyboard._set_key(name, button);
}