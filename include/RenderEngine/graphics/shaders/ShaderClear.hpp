#pragma once
#include <RenderEngine/graphics/shaders/Shader.hpp>

namespace RenderEngine
{
    class ShaderClear : public Shader
    {
    public:
        ShaderClear() = delete;
        ShaderClear(const ShaderClear& other) = delete;
        ShaderClear(const GPU* gpu);
        ~ShaderClear();
    };
}
