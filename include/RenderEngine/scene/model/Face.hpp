#pragma once
#include <RenderEngine/graphics/Color.hpp>
#include <RenderEngine/graphics/shaders/Types.hpp>
#include <RenderEngine/scene/model/UV.hpp>
#include <RenderEngine/scene/model/Material.hpp>
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
        Face(const std::array<Vector, 3>& _points, const std::array<UV, 3>& UVs, const Color& _color, const Material& _material);
        ~Face();
    public:
        std::array<Vector, 3> points;
        std::array<Vector, 3> normals;
        std::array<UV, 3> UVs;
        Color color;
        Material material;
    public:
        static std::vector<Face> quad(const Vector& p1, const Vector& p2, const Vector& p3, const Vector& p4, const Color& color, const Material& material={});
        static std::vector<Face> cube(double length, const std::optional<Color>& color=std::nullopt, const Material& material={});  // Creates a cube with side length 'length'
        static std::vector<Face> sphere(double radius, uint32_t divides=4, bool smooth_normals=true, const std::optional<Color>& color=std::nullopt, const Material& material={});  // Create a sphere by dividing 'divides' times the sides of the triangular faces of an icosahedron.
        static std::vector<Face> cylinder(double length, double radius, uint32_t divides=10, bool smooth_normals=true, const std::optional<Color>& color=std::nullopt, const Material& material={});
        static std::vector<Face> cone(double length, double radius, uint32_t divides=10, bool smooth_normals=true, const std::optional<Color>& color=std::nullopt, const Material& material={});
    protected:
        static Vector _spherical_to_cartesian(const Vector& p);
        static Vector _cartesian_to_spherical(const Vector& p);
    };
}