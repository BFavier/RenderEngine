#include <iostream>
#include <RenderEngine/render_engine.hpp>
#include <vector>
#include <iostream>
using namespace RenderEngine;

int main()
{
    Engine::initialize({"VK_LAYER_KHRONOS_validation"});
    GPU gpu = GPU::get_best_device();
    WindowSettings settings;
    settings.transparent = true;
    std::cout << "Creating window" << std::endl;
    Window window(gpu, settings);
    Mouse& mouse = window.mouse;
    while(!window.closing())
    {
        double dx = mouse.dx();
        if (dx != 0)
        {
            std::cout << dx << std::endl;
        }
        window.update();
    }
    return EXIT_SUCCESS;
}
