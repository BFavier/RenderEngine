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

    #pragma pack(push, 1)  // removes padding in binary representation from structs defined below

    struct vec2
    {
        float x;
        float y;
    };

    struct vec3
    {
        float x;
        float y;
        float z;
    };

    struct vec4
    {
        float r;
        float g;
        float b;
        float a;
    };

    struct mat3
    {
        float i1j1;
        float i1j2;
        float i1j3;
        float pad1;

        float i2j1;
        float i2j2;
        float i2j3;
        float pad2;

        float i3j1;
        float i3j2;
        float i3j3;
        float pad3;
    };

    struct Vertex
    {
        vec3 position;  // x, y, z
        vec3 normal;  // nx, ny, nz
        vec4 color;  // r, g, b, a
    };

    struct MeshDrawParameters
    {
        vec4 mesh_position;
        mat3 mesh_rotation;
        vec4 camera_parameters;
        float mesh_scale;
    };

    struct CameraParameters
    {
        vec4 camera_position;
        mat3 world_to_camera;
        vec2 camera_aperture_size;
        float focal_length;
        float camera_scale;
    };

    #pragma pack(pop)
}