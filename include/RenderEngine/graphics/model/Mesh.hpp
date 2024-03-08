#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <RenderEngine/graphics/model/Face.hpp>


namespace RenderEngine
{
    class Mesh
    {
    public:
        Mesh() = delete;
        Mesh(const std::shared_ptr<GPU>& gpu, const std::vector<Face>& faces, std::array<float, 4> color);
        ~Mesh();
    public:
        const std::shared_ptr<GPU>& gpu;
        std::shared_ptr<VkBuffer> _vk_buffer = nullptr;
    public:
        static Mesh cube(vec3 center, float radius);
    };    
}