#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <string>

namespace RenderEngine
{
    enum ImageFormat {RGBA=VK_FORMAT_R8G8B8A8_SRGB, // Format to store gamma corrected (sRGB) image colors
                      NORMAL=VK_FORMAT_R8G8B8A8_SNORM, // Format to store normals data in the [-1, 1] range
                      MATERIAL=VK_FORMAT_R8G8B8A8_UNORM, // Format to store material data in the [0, 1] range
                      FLOAT4=VK_FORMAT_R32G32B32A32_SFLOAT, // 4 floats
                      DEPTH=0 // will be replaced with best supported depth format
                      };
}