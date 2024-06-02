#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <RenderEngine/graphics/model/Face.hpp>
#include <RenderEngine/graphics/Buffer.hpp>


namespace RenderEngine
{
    class Mesh
    {
        friend class Canvas;
    public:
        Mesh() = delete;
        Mesh(const std::shared_ptr<GPU>& gpu, const std::vector<Face>& faces);
        ~Mesh();
    public:
        void upload(const std::vector<Face>& faces);
        std::size_t bytes_size() const;
    public:
        static std::vector<std::shared_ptr<Mesh>> bulk_allocate_meshes(const std::vector<std::vector<Face>>& faces);
    protected:
        std::shared_ptr<Buffer> _buffer = nullptr;
        std::size_t _offset = 0;
        std::size_t _bytes_size;
    };
}
