#pragma once
#include <RenderEngine/utilities/External.hpp>

namespace RenderEngine
{
    enum Type{FLOAT=VK_FORMAT_R32_SFLOAT,
              VEC2=VK_FORMAT_R32G32_SFLOAT,
              VEC3=VK_FORMAT_R32G32B32_SFLOAT,
              VEC4=VK_FORMAT_R32G32B32A32_SFLOAT,
              INT=VK_FORMAT_R32_SINT,
              IVEC2=VK_FORMAT_R32G32_SINT,
              IVEC3=VK_FORMAT_R32G32B32_SINT,
              IVEC4=VK_FORMAT_R32G32B32A32_SINT,
              UINT=VK_FORMAT_R32_UINT,
              UVEC2=VK_FORMAT_R32G32_UINT,
              UVEC3=VK_FORMAT_R32G32B32_UINT,
              UVEC4=VK_FORMAT_R32G32B32A32_UINT,
              DOUBLE=VK_FORMAT_R64_SFLOAT,
              DVEC2=VK_FORMAT_R64G64_SFLOAT,
              DVEC3=VK_FORMAT_R64G64B64_SFLOAT,
              DVEC4=VK_FORMAT_R64G64B64A64_SFLOAT};
    
    struct vec3
    {
        float x, y, z;

        vec3 operator+(const vec3& other) const
        {
            return {x + other.x, y + other.y, z + other.z};
        }

        vec3 operator*(float factor) const
        {
            return {x*factor, y*factor, z*factor};
        }
    };

    struct vec4
    {
        float r, g, b, a;

        vec4 operator+(const vec4& other) const
        {
            return {r + other.r, g + other.g, b + other.b, a + other.a};
        }

        vec4 operator*(float factor) const
        {
            return {r*factor, g*factor, b*factor, a*factor};
        }
    };
}