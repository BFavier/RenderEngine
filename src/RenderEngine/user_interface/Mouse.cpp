#include <RenderEngine/user_interface/Mouse.hpp>
#include <RenderEngine/user_interface/Window.hpp>
#include <RenderEngine/utilities/Macro.hpp>
using namespace RenderEngine;

Mouse::Mouse(const Window& window) : _window(window)
{
}

Mouse::~Mouse()
{
}

const Button& Mouse::button(const std::string& button_name) const
{
    if (_buttons.find(button_name) == _buttons.end())
    {
        THROW_ERROR("The button " + button_name + " doesn't exist.");
    }
    return _buttons.at(button_name);
}

const std::map<const std::string, Button>& Mouse::buttons() const
{
    return _buttons;
}

unsigned int Mouse::x() const
{
    return _x;
}

unsigned int Mouse::y() const
{
    return _y;
}

int Mouse::dx() const
{
    return _dx;
}

int Mouse::dy() const
{
    return _dy;
}

double Mouse::dx_rel() const
{
    return static_cast<double>(dx()) / std::max(1U, _window.width() - 1);
}

double Mouse::dy_rel() const
{
    return static_cast<double>(dy()) / std::max(1U, _window.height() - 1);
}

double Mouse::x_rel() const
{
    return static_cast<double>(x()) / std::max(1U, _window.width() - 1);
}

double Mouse::y_rel() const
{
    return static_cast<double>(y()) / std::max(1U, _window.height() - 1);
}

double Mouse::wheel_dx() const
{
    return _wheel_dx;
}

double Mouse::wheel_dy() const
{
    return _wheel_dy;
}

bool Mouse::hidden() const
{
    return _mouse_hidden;
}

void Mouse::hide(bool h)
{
    if (h)
    {
        glfwSetInputMode(_window._glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else
    {
        glfwSetInputMode(_window._glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    _mouse_hidden = h;
}

void Mouse::_set_button(const std::string& name, const Button& button)
{
    _buttons[name] = button;
}
