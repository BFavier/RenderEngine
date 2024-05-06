#pragma once
#include <RenderEngine/graphics/shaders/Shader.hpp>

namespace RenderEngine
{
    class Shader3D : public Shader
    {
    public:
        Shader3D() = delete;
        Shader3D(const Shader3D& other) = delete;
        Shader3D(const GPU* gpu);
        ~Shader3D();
    };
}
