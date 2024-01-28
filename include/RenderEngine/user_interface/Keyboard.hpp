#pragma once
#include <map>
#include <string>
#include <memory>
#include <GLFW/glfw3.h>
#include <RenderEngine/user_interface/Button.hpp>

namespace RenderEngine
{
    class Window;

    class Keyboard
    {

    friend class Window;
    
    public:
        Keyboard() = delete;
        Keyboard(const Window& window);
        ~Keyboard();
    public:
        const std::map<const std::string, Button>& keys() const;
        const Button& key(const std::string& name) const;
    protected:
        std::string _get_key_name(int key, int scancode);
        void _set_key(const std::string& name, const Button& key);
    protected:
        const Window& _window;
        std::map<const std::string, Button> _keys;
    };
}
