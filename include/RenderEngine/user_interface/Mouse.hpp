#pragma once
#include <map>
#include <memory>
#include <string>
#include <GLFW/glfw3.h>
#include <RenderEngine/user_interface/Button.hpp>

namespace RenderEngine
{
    class Window;

    class Mouse
    {
    
    friend class Window;

    public:
        Mouse() = delete;
        Mouse(const Window& window);
        ~Mouse();
    public:
        const Button& button(const std::string& button_name) const;
        const std::map<const std::string, Button>& buttons() const;
        unsigned int x() const;
        unsigned int y() const;
        int dx() const;
        int dy() const;
        double wheel_dx() const;
        double wheel_dy() const;
        double x_rel() const;
        double y_rel() const;
        bool hidden() const;
        void hide(bool hide);
    protected:
        void _set_button(const std::string& name, const Button& button);
    protected:
        unsigned int _x;
        unsigned int _y;
        int _dx;
        int _dy;
        double _wheel_dx;
        double _wheel_dy;
        const Window& _window;
        bool _mouse_hidden = false;
        std::map<const std::string, Button> _buttons;
    };
}
