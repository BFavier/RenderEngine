#pragma once
#include <RenderEngine/graphics/shaders/Shader.hpp>

namespace RenderEngine
{
    class DemoShader : public Shader
    {
    public:
        DemoShader() = delete;
        DemoShader(const DemoShader& other) = delete;
        DemoShader(const GPU* gpu);
        ~DemoShader();
    };
}
