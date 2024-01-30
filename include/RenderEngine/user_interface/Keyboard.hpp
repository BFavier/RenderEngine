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
    
    public: // this class is non copyable
        Keyboard() = delete;
        Keyboard(const Keyboard& other) = delete;
        Keyboard& operator=(const Keyboard& other) = delete;
    public:
        Keyboard(const Window& window);
        ~Keyboard();
    public:
        //< returns a mapping of {key name: key status}
        const std::map<const std::string, Button>& keys() const;
        //< returns the status of the given key, throws an error if it is not present in the mapping
        const Button& key(const std::string& name) const;
    protected:
        std::string _get_key_name(int key, int scancode);
        void _set_key(const std::string& name, const Button& key);
    protected:
        const Window& _window;
        std::map<const std::string, Button> _keys;
    };
}
