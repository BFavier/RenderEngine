#include <iostream>
#include <RenderEngine/render_engine.hpp>
#include <RenderEngine/utilities/Macro.hpp>  // For PI
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
        std::map<std::string, std::shared_ptr<Image>> images = Image::bulk_load_images(gpu, std::map<std::string, std::string>({{"dices", "./dices.png"}, {"screenshot", "./screenshot.png"}}), ImageFormat::RGBA, 1024, 1024);
        images["dices"]->save_to_disk("./dices_loaded.png");
        images["screenshot"]->save_to_disk("./screenshot_loaded.png");
        std::map<std::string, std::shared_ptr<Mesh>> meshes = Mesh::bulk_allocate_meshes(gpu,
            {{"cube", Face::cube(0.5)},
             {"cone", Face::cone(0.5, 0.1, 20)},
             {"quad", Face::quad(Vector(1., 0., 1.), Vector(-1., 0., 1.), Vector(-1., 0., -1.), Vector(1., 0., -1.), Color(1.0, 1.0, 1.0, 1.0))}});
        Model cube(meshes["cube"], Vector(0., -1., 0.));
        Model floor(meshes["quad"], Vector(0., 0., 0.), Quaternion(), 5.0);
        Referential yaw(Vector(0., -1.0, -1.), Quaternion(), 1.0);  // yaw only rotates around the global Y axis
        Referential pitch(Vector(), Quaternion(), 1.0, &yaw);  // pitch only rotates around the yaw's X axis
        PerspectiveCamera camera(PI/2, 0.1, 1000., Vector(), Quaternion(), 1.0, &pitch); // the camera only pitches around yaw's X axis
        OrthographicCamera ortho_camera(5.0, 1000., Vector(), Quaternion(), 1.0, &pitch); // the camera only pitches around yaw's X axis
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
                yaw.orientation = yaw.orientation * Quaternion(mouse.dx_rel()*PI, Vector(0.0, 1.0, 0.0));
                pitch.orientation = pitch.orientation * Quaternion(mouse.dy_rel()*PI, Vector(-1.0, 0.0, 0.0));
            }
            if (keyboard.keys().at("PRINT SCREEN").was_released)
            {
                Canvas* frame = window.current_frame();
                if (frame != nullptr)
                {
                    frame->wait_completion();
                    frame->normal.save_to_disk("screenshot.png");
                    std::cout << "screenshot saved." << std::endl;
                }
            }
            Canvas* frame = window.next_frame();
            if (frame != nullptr)
            {
                frame->clear(Color(0.1, 0.0, 0.3, 1.0));
                // frame->set_view(camera);
                frame->draw(camera, cube.mesh, cube.coordinates_in(camera), true);
                frame->draw(camera, floor.mesh, floor.coordinates_in(camera), false);
            }
            window.update();
        }
    }
    Internal::terminate();
    return EXIT_SUCCESS;
}
