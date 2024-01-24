#include <RenderEngine/user_interface/Mouse.hpp>
#include <RenderEngine/user_interface/Window.hpp>
using namespace RenderEngine;

Mouse::Mouse(const Window& window)
{
    _state = window._get_state();
}

Mouse::Mouse(const Mouse& other)
{
    *this = other;
}

Mouse::~Mouse()
{
}

const Button& Mouse::button(const std::string& button_name) const
{
    const std::map<const std::string, Button>& buttons = _state->_mouse_buttons;
    if (buttons.find(button_name) == buttons.end())
    {
        throw std::runtime_error("The button " + button_name + " doesn't exist.");
    }
    return buttons.at(button_name);
}

const std::map<const std::string, Button>& Mouse::buttons() const
{
    return _state->_mouse_buttons;
}

double Mouse::x() const
{
    return _state->_mouse_x;
}

double Mouse::y() const
{
    return _state->_mouse_y;
}

double Mouse::dx() const
{
    return _state->_mouse_dx;
}

double Mouse::dy() const
{
    return _state->_mouse_dy;
}

double Mouse::wheel_x() const
{
    return _state->_mouse_wheel_x;
}

double Mouse::wheel_y() const
{
    return _state->_mouse_wheel_y;
}

bool Mouse::hidden() const
{
    return _state->_mouse_hidden;
}

void Mouse::hide(bool h)
{
    if (h)
    {
        glfwSetInputMode(_state->_glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else
    {
        glfwSetInputMode(_state->_glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    _state->_mouse_hidden = h;
}

const Mouse& Mouse::operator=(const Mouse& other)
{
    _state = other._get_state();
    return *this;
}

std::shared_ptr<WindowState> Mouse::_get_state() const
{
    return _state;
}

void Mouse::_set_state(const std::shared_ptr<WindowState>& state)
{
    _state = state;
}