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
        static std::vector<Face> quad(const Vector& p1, const Vector& p2, const Vector& p3, const Vector& p4);
        static std::vector<Face> cube(double length);  // Creates a cube with side length 'length'
        static std::vector<Face> sphere(double radius, uint32_t divides=2);  // Create a sphere by dividing 'divides' times the sides of the triangular faces of an icosahedron.
        static std::vector<Face> cylinder(float length);
        static std::vector<Face> cone(float length);
    protected:
        std::shared_ptr<Buffer> _buffer = nullptr;
        std::size_t _offset = 0;
        std::size_t _bytes_size;
    protected:
        static Vector _spherical_to_cartesian(const Vector& p);
    };
}
