#pragma once
#include <RenderEngine/graphics/shaders/Types.hpp>
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
            float v1_x = _points[1].x - _points[0].x;
            float v1_y = _points[1].y - _points[0].y;
            float v1_z = _points[1].z - _points[0].z;
            float v2_x = _points[2].x - _points[0].x;
            float v2_y = _points[2].y - _points[0].y;
            float v2_z = _points[2].z - _points[0].z;
            normal.x = v1_y*v2_z-v1_z*v2_y;
            normal.y = v1_z*v2_x-v1_x*v2_z;
            normal.z = v1_x*v2_y-v1_y*v2_x;
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