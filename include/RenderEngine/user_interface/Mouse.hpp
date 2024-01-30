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

    public: // this class is non copyable
        Mouse() = delete;
        Mouse(const Mouse& other) = delete;
        Mouse& operator=(const Mouse& other) = delete;
    public:
        Mouse(const Window& window);
        ~Mouse();
    public:
        //< returns a mapping of {button name: button status}
        const Button& button(const std::string& button_name) const;
        //< returns the status of a given button, throw an error if it is not present in the mapping
        const std::map<const std::string, Button>& buttons() const;
        //< returns mouse x positions in the window
        unsigned int x() const;
        //< returns mouse y positions in the window
        unsigned int y() const;
        //< returns mouse horizontal displacement in the window since previous update
        int dx() const;
        //< returns mouse vertical displacement in the window since previous update
        int dy() const;
        //< returns the mouse wheel horizontal displacement
        double wheel_dx() const;
        //< returns the mouse wheel vertical displacement
        double wheel_dy() const;
        //< returns the relative x position of the mouse in the window (0.0 on first pixel, 1.0 on last)
        double x_rel() const;
        //< returns the relative y position of the mouse in the window (0.0 on first pixel, 1.0 on last)
        double y_rel() const;
        //< returns whether the mouse is in hidden state
        bool hidden() const;
        //< enables/disable mouse hidden state. In hidden state, the mouse is trapped in the window, and only the relative displacements are registered.
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
