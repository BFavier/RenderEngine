#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <RenderEngine/graphics/model/Face.hpp>
#include <RenderEngine/graphics/Buffer.hpp>


namespace RenderEngine
{
    class Mesh : public Buffer
    {
    public:
        Mesh() = delete;
        Mesh(const std::shared_ptr<GPU>& gpu, const std::vector<Face>& faces);
        ~Mesh();
    public:
        static Mesh cube(vec3 center, float radius);
    };    
}