#pragma once
#include <RenderEngine/graphics/shaders/Shader.hpp>

namespace RenderEngine
{
    class ShaderLight : public Shader
    {
    public:
        ShaderLight() = delete;
        ShaderLight(const ShaderLight& other) = delete;
        ShaderLight(const GPU* gpu);
        ~ShaderLight();
    };
}
