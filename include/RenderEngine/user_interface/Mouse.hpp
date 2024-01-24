#pragma once
#include <map>
#include <memory>
#include <GLFW/glfw3.h>
#include "WindowState.hpp"
#include "Button.hpp"

namespace RenderEngine
{
    class Window;

    class Mouse
    {
    public:
        Mouse() = delete;
        Mouse(const Window& window);
        Mouse(const Mouse& other);
        ~Mouse();
    public:
        const Button& button(const std::string& button_name) const;
        const std::map<const std::string, Button>& buttons() const;
        double x() const;
        double y() const;
        double dx() const;
        double dy() const;
        double wheel_x() const;
        double wheel_y() const;
        double x_rel() const;
        double y_rel() const;
        bool hidden() const;
        void hide(bool hide);
    public:
        const Mouse& operator=(const Mouse& other);
    public:
        std::shared_ptr<WindowState> _state;
    };
}
