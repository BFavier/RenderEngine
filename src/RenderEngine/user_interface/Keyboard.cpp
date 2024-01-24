#include <RenderEngine/user_interface/Keyboard.hpp>
#include <RenderEngine/user_interface/Window.hpp>
using namespace RenderEngine;

Keyboard::Keyboard(const Window& window)
{
    _state = window._get_state();
}

Keyboard::Keyboard(const Keyboard& other)
{
    *this = other;
}

Keyboard::~Keyboard()
{
}

const std::map<const std::string, Button>& Keyboard::keys() const
{
    return _state->_keyboard_buttons;
}

const Button& Keyboard::key(const std::string& name) const
{
    const std::map<const std::string, Button>& keys =  _state->_keyboard_buttons;
    if (keys.find(name) == keys.end())
    {
        throw std::runtime_error("The key '" + name + "' is unknown.");
    }
    return keys.at(name);
}

const Keyboard& Keyboard::operator=(const Keyboard& other)
{
    _state = other._state;
    return *this;
}
