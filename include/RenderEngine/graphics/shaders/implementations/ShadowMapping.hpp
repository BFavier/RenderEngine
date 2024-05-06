#pragma once
#include <RenderEngine/graphics/shaders/ComputeShader.hpp>

namespace RenderEngine
{
    class ShadowMapping : public ComputeShader
    {
    public:
        ShadowMapping() = delete;
        ShadowMapping(const ShadowMapping& other) = delete;
        ShadowMapping(const GPU* gpu);
        ~ShadowMapping();
    };
}
