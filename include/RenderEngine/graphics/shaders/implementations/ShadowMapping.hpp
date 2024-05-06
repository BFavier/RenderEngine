#pragma once
#include <RenderEngine/graphics/shaders/Shader.hpp>

namespace RenderEngine
{
    class ShadowMapping : public Shader
    {
    public:
        ShadowMapping() = delete;
        ShadowMapping(const ShadowMapping& other) = delete;
        ShadowMapping(const GPU* gpu);
        ~ShadowMapping();
    };
}
