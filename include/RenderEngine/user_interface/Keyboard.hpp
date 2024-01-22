#pragma once
#include <map>
#include <string>
#include <memory>
#include <GLFW/glfw3.h>
#include "Handles.hpp"
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
        const std::shared_ptr<Handles>& _get_state() const;
        void _set_state(const std::shared_ptr<Handles>& state);
    protected:
        std::shared_ptr<Handles> _state = nullptr;
    };
}
