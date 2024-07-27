#pragma once
#include <RenderEngine/graphics/shaders/Shader.hpp>

namespace RenderEngine
{
    class ShaderShadow : public Shader
    {
    public:
        ShaderShadow() = delete;
        ShaderShadow(const ShaderShadow& other) = delete;
        ShaderShadow(const GPU* gpu);
        ~ShaderShadow();
    };
}
