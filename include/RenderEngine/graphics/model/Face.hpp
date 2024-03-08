#pragma once
#include <RenderEngine/graphics/shaders/Type.hpp>
#include <array>
#include <cmath>

namespace RenderEngine
{
    struct Face
    {
        Face() = delete;
        Face(const std::array<vec3, 3>& _points, const vec4& _color)
        {
            points = _points;
            color = _color;
            float u1 = _points[1].x - _points[0].x;
            float u2 = _points[1].y - _points[0].y;
            float u3 = _points[1].z - _points[0].z;
            float v1 = _points[2].x - _points[0].x;
            float v2 = _points[2].y - _points[0].y;
            float v3 = _points[2].z - _points[0].z;
            normal.x = u2*v3-u3*v2;
            normal.y = u3*v1-u1*v3;
            normal.z = u1*v2-u2*v1;
            float scale = std::pow(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z, 0.5);
            normal.x = normal.x / scale;
            normal.y = normal.y / scale;
            normal.z = normal.z / scale;
        };
        ~Face(){};
        std::array<vec3, 3> points;
        vec3 normal;
        vec4 color;
    };   
}