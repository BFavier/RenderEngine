#include <RenderEngine/user_interface/WindowState.hpp>
#include <RenderEngine/Internal.hpp>

using namespace RenderEngine;

WindowState::WindowState()
{
    WindowSettings settings;
    _initialize(settings);
}

WindowState::WindowState(const std::string& title, unsigned int width, unsigned int height)
{
    WindowSettings settings;
    settings.title = title;
    settings.width = width;
    settings.height = height;
    _initialize(settings);
}

WindowState::WindowState(const WindowSettings& settings)
{
    _initialize(settings);
}

WindowState::~WindowState()
{
    glfwDestroyWindow(_glfw_window);
    vkDestroySurfaceKHR(Internal::get_vulkan_instance(), _vk_surface, nullptr);
}

void WindowState::_window_resize_callback(GLFWwindow* window, int width, int height)
{
    WindowState* h = static_cast<WindowState*>(glfwGetWindowUserPointer(window));
    h->_window_width = width;
    h->_window_height = height;
    // glfwSetWindowSize(h->_glfw_window, width, height);
}

void WindowState::_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    (void)mods;//Silence the annoying unused parameter warning
    WindowState* h = static_cast<WindowState*>(glfwGetWindowUserPointer(window));
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
    Button& status = h->_mouse_buttons[name];
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
}

void WindowState::_mouse_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    WindowState* h = static_cast<WindowState*>(glfwGetWindowUserPointer(window));
    h->_mouse_dx = xpos - h->_mouse_x;
    h->_mouse_dy = ypos - h->_mouse_y;
    h->_mouse_x = xpos;
    h->_mouse_y = ypos;
}

void WindowState::_mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    WindowState* h = static_cast<WindowState*>(glfwGetWindowUserPointer(window));
    h->_mouse_wheel_x = xoffset;
    h->_mouse_wheel_y = yoffset;
}

void WindowState::_keyboard_button_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)mods;//Silence the annoying unused parameter warning
    WindowState* h = static_cast<WindowState*>(glfwGetWindowUserPointer(window));
    std::string name = _get_key_name(key, scancode);
    Button& button = h->_keyboard_buttons[name];
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
}

std::string WindowState::_get_key_name(int key, int scancode)
{
    switch (key)
    {
        case GLFW_KEY_1:            return "1";
        case GLFW_KEY_2:            return "2";
        case GLFW_KEY_3:            return "3";
        case GLFW_KEY_4:            return "4";
        case GLFW_KEY_5:            return "5";
        case GLFW_KEY_6:            return "6";
        case GLFW_KEY_7:            return "7";
        case GLFW_KEY_8:            return "8";
        case GLFW_KEY_9:            return "9";
        case GLFW_KEY_0:            return "0";
        case GLFW_KEY_SPACE:        return "SPACE";
        case GLFW_KEY_WORLD_1:      return "WORLD 1";
        case GLFW_KEY_WORLD_2:      return "WORLD 2";
        case GLFW_KEY_ESCAPE:       return "ESCAPE";
        case GLFW_KEY_F1:           return "F1";
        case GLFW_KEY_F2:           return "F2";
        case GLFW_KEY_F3:           return "F3";
        case GLFW_KEY_F4:           return "F4";
        case GLFW_KEY_F5:           return "F5";
        case GLFW_KEY_F6:           return "F6";
        case GLFW_KEY_F7:           return "F7";
        case GLFW_KEY_F8:           return "F8";
        case GLFW_KEY_F9:           return "F9";
        case GLFW_KEY_F10:          return "F10";
        case GLFW_KEY_F11:          return "F11";
        case GLFW_KEY_F12:          return "F12";
        case GLFW_KEY_F13:          return "F13";
        case GLFW_KEY_F14:          return "F14";
        case GLFW_KEY_F15:          return "F15";
        case GLFW_KEY_F16:          return "F16";
        case GLFW_KEY_F17:          return "F17";
        case GLFW_KEY_F18:          return "F18";
        case GLFW_KEY_F19:          return "F19";
        case GLFW_KEY_F20:          return "F20";
        case GLFW_KEY_F21:          return "F21";
        case GLFW_KEY_F22:          return "F22";
        case GLFW_KEY_F23:          return "F23";
        case GLFW_KEY_F24:          return "F24";
        case GLFW_KEY_F25:          return "F25";
        case GLFW_KEY_UP:           return "UP";
        case GLFW_KEY_DOWN:         return "DOWN";
        case GLFW_KEY_LEFT:         return "LEFT";
        case GLFW_KEY_RIGHT:        return "RIGHT";
        case GLFW_KEY_LEFT_SHIFT:   return "SHIFT";
        case GLFW_KEY_RIGHT_SHIFT:  return "RIGHT SHIFT";
        case GLFW_KEY_LEFT_CONTROL: return "CONTROL";
        case GLFW_KEY_RIGHT_CONTROL: return "RIGHT CONTROL";
        case GLFW_KEY_LEFT_ALT:     return "ALT";
        case GLFW_KEY_RIGHT_ALT:    return "RIGHT ALT";
        case GLFW_KEY_TAB:          return "TAB";
        case GLFW_KEY_ENTER:        return "ENTER";
        case GLFW_KEY_BACKSPACE:    return "BACKSPACE";
        case GLFW_KEY_INSERT:       return "INSERT";
        case GLFW_KEY_DELETE:       return "DELETE";
        case GLFW_KEY_PAGE_UP:      return "PAGE UP";
        case GLFW_KEY_PAGE_DOWN:    return "PAGE DOWN";
        case GLFW_KEY_HOME:         return "HOME";
        case GLFW_KEY_END:          return "END";
        case GLFW_KEY_KP_0:         return "KEYPAD 0";
        case GLFW_KEY_KP_1:         return "KEYPAD 1";
        case GLFW_KEY_KP_2:         return "KEYPAD 2";
        case GLFW_KEY_KP_3:         return "KEYPAD 3";
        case GLFW_KEY_KP_4:         return "KEYPAD 4";
        case GLFW_KEY_KP_5:         return "KEYPAD 5";
        case GLFW_KEY_KP_6:         return "KEYPAD 6";
        case GLFW_KEY_KP_7:         return "KEYPAD 7";
        case GLFW_KEY_KP_8:         return "KEYPAD 8";
        case GLFW_KEY_KP_9:         return "KEYPAD 9";
        case GLFW_KEY_KP_DIVIDE:    return "KEYPAD /";
        case GLFW_KEY_KP_MULTIPLY:  return "KEYPAD *";
        case GLFW_KEY_KP_SUBTRACT:  return "KEYPAD -";
        case GLFW_KEY_KP_ADD:       return "KEYPAD +";
        case GLFW_KEY_KP_DECIMAL:   return "KEYPAD .";
        case GLFW_KEY_KP_EQUAL:     return "KEYPAD =";
        case GLFW_KEY_KP_ENTER:     return "KEYPAD ENTER";
        case GLFW_KEY_PRINT_SCREEN: return "PRINT SCREEN";
        case GLFW_KEY_NUM_LOCK:     return "NUM LOCK";
        case GLFW_KEY_CAPS_LOCK:    return "CAPS LOCK";
        case GLFW_KEY_SCROLL_LOCK:  return "SCROLL LOCK";
        case GLFW_KEY_PAUSE:        return "PAUSE";
        case GLFW_KEY_LEFT_SUPER:   return "LEFT SUPER";
        case GLFW_KEY_RIGHT_SUPER:  return "RIGHT SUPER";
        case GLFW_KEY_MENU:         return "MENU";
        default:                    break;
    }
    const char* name = glfwGetKeyName(key, scancode);
    if (name != nullptr)
    {
        std::string lower(name);
        std::string upper;
        upper.reserve(lower.size());
        for (char c : lower)
        {
            upper.push_back(std::toupper(c));
        }
        return upper;
    }
    return "UNKNOWN";
}


