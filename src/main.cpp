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
        Referential model;
        // Face face({{ {0.f, 0.5f, 0.f }, { -0.5, -0.5, 0. }, { 0.5, -0.5, 0. } }}, { 1.0, 0., 0., 1.0 });
        // Mesh cube(gpu, { face });
        Referential yaw(Vector(0., 0., -1.), Quaternion(), 1.0);  // yaw only rotates around the global Y axis
        Camera camera(90.0, Vector(), Quaternion(), 1.0, &yaw); // the camera only pitches around yaw's X axis
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
                camera.orientation = camera.orientation * Quaternion(mouse.dy_rel()*180, { -1.0, 0.0, 0.0 });
            }
            if (keyboard.keys().at("PRINT SCREEN").was_released)
            {
                Canvas* frame = window.current_frame();
                if (frame != nullptr)
                {
                    frame->wait_completion();
                    frame->color.save_to_disk("screenshot.png");
                    std::cout << "screenshot saved." << std::endl;
                }
            }
            Canvas* frame = window.next_frame();
            if (frame != nullptr)
            {
                std::tie(camera.aperture_width, camera.aperture_height) = std::make_tuple(frame->color.width() * 0.001, frame->color.height() * 0.001);
                frame->clear(10, 0, 30, 255);
                frame->set_view(camera);
                frame->draw(cube, model.coordinates_in(camera), false);
            }
            window.update();
        }
    }
    Internal::terminate();
    return EXIT_SUCCESS;
}
