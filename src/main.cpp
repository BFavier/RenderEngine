#include <iostream>
#include <RenderEngine/render_engine.hpp>
#include <vector>
#include <iostream>
using namespace RenderEngine;

int main()
{
    Internal::initialize({"VK_LAYER_KHRONOS_validation"});
    {
        std::shared_ptr<GPU> gpu = Internal::get_best_GPU();
        WindowSettings settings;
        Window window(gpu, settings);
        Mouse& mouse = window.mouse;
        Keyboard& keyboard = window.keyboard;
        Timer timer;
        Mesh cube = Mesh::cube(gpu, 0.5);
        Referential yaw(nullptr, {0., 0., 0.}, {45.0, {0., 1., 0.}});
        Referential pitch(&yaw, {0., 0., 0.}, {});
        Camera camera(gpu, 0.16, 0.09, 90.0, &pitch, {0., 0., -1.});
        while(!window.closing())
        {
            double dt = timer.dt();
            if ((mouse.dx() != 0) || (mouse.dy() != 0))
            {
                std::cout << "mouse position changed to " << mouse.x() << ", " << mouse.y() << " (" << dt << ") " << std::endl;
            }
            if (mouse.wheel_dy() != 0)
            {
                std::cout << "mouse sheel moved of " << mouse.wheel_dy() << " (" << dt << ") " << std::endl;
            }
            for (std::pair<std::string, const Button&> button : mouse.buttons())
            {
                if (button.second.was_pressed || button.second.was_released)
                {
                    std::cout << button.first << " state changed to pressed=" << button.second.down << " (" << dt << ") " << std::endl;
                }
            }
            for (std::pair<std::string, const Button&> key : keyboard.keys())
            {
                if (key.second.was_pressed || key.second.was_released)
                {
                    std::cout << key.first << " key state changed to pressed=" << key.second.down << " (" << dt << ") " << std::endl;
                }
            }
            if (keyboard.keys().at("Z").down)
            {
                yaw.position += yaw.orientation * Vector(0., 0., 1.0*dt);
            }
            if (keyboard.keys().at("S").down)
            {
                yaw.position -= yaw.orientation * Vector(0., 0., 1.0*dt);
            }
            if (keyboard.keys().at("Q").down)
            {
                yaw.position -= yaw.orientation * Vector(1.0*dt, 0., 0.);
            }
            if (keyboard.keys().at("D").down)
            {
                yaw.position += yaw.orientation * Vector(1.0*dt, 0., 0.);
            }
            if (mouse.buttons().at("LEFT CLICK").down)
            {
                yaw.orientation = yaw.orientation * Quaternion(mouse.dx_rel()*180, { 0.0, 1.0, 0.0 });
                pitch.orientation = pitch.orientation * Quaternion(mouse.dy_rel()*180, { -1.0, 0.0, 0.0 });
            }
            Canvas* frame = window.next_frame();
            if (frame != nullptr)
            {
                frame->clear(10, 0, 30, 255);
                frame->bind_camera(camera);
                frame->draw(cube, {0., 0., 0.}, {});
            }
            window.update();
        }
    }
    Internal::terminate();
    return EXIT_SUCCESS;
}
