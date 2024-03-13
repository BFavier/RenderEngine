#include <iostream>
#include <RenderEngine/render_engine.hpp>
#include <vector>
#include <iostream>
using namespace RenderEngine;

int main()
{
    Internal::initialize({"VK_LAYER_KHRONOS_validation"},
                         {VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
    {
        std::shared_ptr<GPU> gpu = Internal::get_best_GPU();
        WindowSettings settings;
        Window window(gpu, settings);
        Mouse& mouse = window.mouse;
        Keyboard& keyboard = window.keyboard;
        Timer timer;
        Mesh cube = Mesh::cube(gpu, 0.5);
        Quaternion mesh_rotation;
        std::pair<double, double> drag_position = {0., 0.};
        std::pair<double, double> drop_position = {0., 0.};
        while(!window.closing())
        {
            if ((mouse.dx() != 0) || (mouse.dy() != 0))
            {
                std::cout << "mouse position changed to " << mouse.x() << ", " << mouse.y() << " (" << timer.dt() << ") " << std::endl;
            }
            if (mouse.wheel_dy() != 0)
            {
                std::cout << "mouse sheel moved of " << mouse.wheel_dy() << " (" << timer.dt() << ") " << std::endl;
            }
            for (std::pair<std::string, const Button&> button : mouse.buttons())
            {
                if (button.second.was_pressed || button.second.was_released)
                {
                    std::cout << button.first << " state changed to pressed=" << button.second.down << " (" << timer.dt() << ") " << std::endl;
                }
            }
            for (std::pair<std::string, const Button&> key : keyboard.keys())
            {
                if (key.second.was_pressed || key.second.was_released)
                {
                    std::cout << key.first << " key state changed to pressed=" << key.second.down << " (" << timer.dt() << ") " << std::endl;
                }
            }
            if (mouse.buttons().at("LEFT CLICK").was_pressed)
            {
                drag_position = {mouse.x_rel(), mouse.y_rel()};
            }
            else if (mouse.buttons().at("LEFT CLICK").was_released)
            {
                drop_position = {mouse.x_rel(), mouse.y_rel()};
                mesh_rotation = Quaternion((drag_position.first - drop_position.first)*180, { 0.0, -1.0, 0.0 }) * mesh_rotation;
                mesh_rotation = Quaternion((drag_position.second - drop_position.second)*180, { 1.0, 0.0, 0.0 }) * mesh_rotation;
            }
            Canvas* frame = window.next_frame();
            if (frame != nullptr)
            {
                frame->draw(cube, mesh_rotation);
            }
            window.update();
        }
    }
    Internal::terminate();
    return EXIT_SUCCESS;
}
