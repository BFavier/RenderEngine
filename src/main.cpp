#include <iostream>
#include <RenderEngine/render_engine.hpp>
#include <vector>
#include <iostream>
using namespace RenderEngine;

int main()
{
    std::vector<std::string> available_validation_layers = Engine::get_available_validation_layers();
    for (const std::string& layer : available_validation_layers)
    {
        std::cout << layer << std::endl;
    }
    Engine::initialize();//{"VK_LAYER_KHRONOS_validation"});
    std::cout << "selecting GPU" << std::endl;
    std::vector<GPU> gpus = GPU::get_devices();
    std::cout << "found " << gpus.size() << " gpus" << std::endl;
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
