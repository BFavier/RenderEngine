#pragma once
#include <RenderEngine/utilities/External.hpp>

namespace RenderEngine
{
    enum ImageFormat {GRAY=VK_FORMAT_R8_SRGB,
                UV=VK_FORMAT_R8G8_SRGB,
                RGB=VK_FORMAT_R8G8B8_SRGB,
                RGBA=VK_FORMAT_R8G8B8A8_SRGB,
                POINTER=VK_FORMAT_R32G32_UINT,
                DEPTH=0 // will be replaced with best supported depth format
                };
}