#pragma once
#include <RenderEngine/graphics/shaders/Types.hpp>
#include <RenderEngine/geometry/Vector.hpp>
#include <array>
#include <cmath>

namespace RenderEngine
{
    struct Face
    {
        Face() = delete;
        Face(const std::array<Vector, 3>& _points, const vec4& _color)
        {
            points = _points;
            color = _color;
            normal = Vector::cross((_points[1] - _points[0]), (_points[2] - _points[0])).normed();
        };
        ~Face(){};
        std::array<Vector, 3> points;
        Vector normal;
        vec4 color;
    };
}