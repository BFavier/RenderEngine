#pragma once
#include <RenderEngine/graphics/shaders/Types.hpp>
#include <RenderEngine/geometry/Vector.hpp>
#include <array>
#include <vector>
#include <optional>
#include <cmath>

namespace RenderEngine
{
    class Face
    {
    public:
        Face() = delete;
        Face(const std::array<Vector, 3>& _points, const vec4& _color);
        ~Face(){};
    public:
        std::array<Vector, 3> points;
        Vector normal;
        vec4 color;
    public:
        static std::vector<Face> quad(const Vector& p1, const Vector& p2, const Vector& p3, const Vector& p4, const vec4& color);
        static std::vector<Face> cube(double length, const std::optional<vec4>& color=std::nullopt);  // Creates a cube with side length 'length'
        static std::vector<Face> sphere(double radius, uint32_t divides=2, const std::optional<vec4>& color=std::nullopt);  // Create a sphere by dividing 'divides' times the sides of the triangular faces of an icosahedron.
        static std::vector<Face> cylinder(double length, double radius, uint32_t divides=10, const std::optional<vec4>& color=std::nullopt);
        static std::vector<Face> cone(double length, double radius, uint32_t divides=10, const std::optional<vec4>& color=std::nullopt);
    protected:
        static Vector _spherical_to_cartesian(const Vector& p);
    };
}