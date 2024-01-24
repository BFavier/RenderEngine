#pragma once
#include <map>
#include <string>
#include <memory>
#include <GLFW/glfw3.h>
#include "WindowState.hpp"
#include "Button.hpp"

namespace RenderEngine
{
    class Window;

    class Keyboard
    {
    public:
        Keyboard() = delete;
        Keyboard(const Window& window);
        Keyboard(const Keyboard& other);
        ~Keyboard();
    public:
        const std::map<const std::string, Button>& keys() const;
        const Button& key(const std::string& name) const;
    public:
        const Keyboard& operator=(const Keyboard& other);
    public:
        std::shared_ptr<WindowState> _state = nullptr;
    };
}
