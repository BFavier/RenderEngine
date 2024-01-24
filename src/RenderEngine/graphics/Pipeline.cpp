#include <RenderEngine/graphics/Pipeline.hpp>
#include <RenderEngine/render_engine.hpp>
using namespace RenderEngine;

Pipeline::Pipeline(const GPU& gpu)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
    if (vkCreatePipelineLayout(*gpu._logical_device, &pipelineLayoutInfo, nullptr, &_vk_graphic_pipeline) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create pipeline layout!")
    }
}

Pipeline::~Pipeline()
{

}