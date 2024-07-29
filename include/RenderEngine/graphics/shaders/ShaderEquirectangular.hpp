#pragma once
#include <RenderEngine/graphics/shaders/Shader.hpp>

namespace RenderEngine
{
    class ShaderEquirectangular : public Shader
    {
    public:
        ShaderEquirectangular() = delete;
        ShaderEquirectangular(const ShaderEquirectangular& other) = delete;
        ShaderEquirectangular(const GPU* gpu);
        ~ShaderEquirectangular();
    };
}
