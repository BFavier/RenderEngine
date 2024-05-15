#pragma once

namespace RenderEngine
{
    enum AntiAliasing {X1=VK_SAMPLE_COUNT_1_BIT,
                       X2=VK_SAMPLE_COUNT_2_BIT,
                       X4=VK_SAMPLE_COUNT_4_BIT,
                       X16=VK_SAMPLE_COUNT_16_BIT,
                       X32=VK_SAMPLE_COUNT_32_BIT,
                       X64=VK_SAMPLE_COUNT_64_BIT};
}