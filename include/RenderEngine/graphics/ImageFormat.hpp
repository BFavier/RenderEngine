#pragma once
#include <RenderEngine/utilities/External.hpp>

namespace RenderEngine
{
    enum ImageFormat {GRAY=VK_FORMAT_R8_SRGB,
                      RGB=VK_FORMAT_R8G8B8_SRGB,
                      RGBA=VK_FORMAT_R8G8B8A8_SRGB,
                      POINTER=VK_FORMAT_R32G32_UINT, // 64 bits unsigned integers for pointers
                      FLOAT3=VK_FORMAT_R32G32B32_SFLOAT, // 3 floats
                      FLOAT4=VK_FORMAT_R32G32B32A32_SFLOAT, // 4 floats
                      DEPTH=0 // will be replaced with best supported depth format
                      };
}