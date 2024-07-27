#pragma once
#include <RenderEngine/graphics/shaders/Shader.hpp>

namespace RenderEngine
{
    class ShaderDemo : public Shader
    {
    public:
        ShaderDemo() = delete;
        ShaderDemo(const ShaderDemo& other) = delete;
        ShaderDemo(const GPU* gpu);
        ~ShaderDemo();
    };
}
