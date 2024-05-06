#include <RenderEngine/graphics/shaders/ComputeShader.hpp>
#include <RenderEngine/graphics/GPU.hpp>
using namespace RenderEngine;

ComputeShader::ComputeShader(const GPU* gpu,
               const std::vector<std::map<std::string, VkDescriptorSetLayoutBinding>>& descriptor_sets, // for each layout set, descriptor of all bindings (textures, Uniform Buffer Objects, ...)
               const std::map<std::string, VkPushConstantRange>& push_constants, // definition of all push constants.
               const std::pair<VkShaderStageFlagBits, std::vector<uint8_t>> stages_bytecode // the bytecode of the compiled spirv file
               )
{
    for (const std::map<std::string, VkDescriptorSetLayoutBinding>& descriptor_set : descriptor_sets)
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (const std::pair<std::string, VkDescriptorSetLayoutBinding>& binding : descriptor_set)
        {
            bindings.push_back(binding.second);
        }
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = bindings.size();
        layoutInfo.pBindings = bindings.data();
        VkDescriptorSetLayout descriptor_set_layout;
        if (vkCreateDescriptorSetLayout(gpu->_logical_device, &layoutInfo, nullptr, &descriptor_set_layout) != VK_SUCCESS)
        {
            THROW_ERROR("failed to create compute descriptor set layout!");
        }
        _descriptor_set_layouts.push_back(descriptor_set_layout);
    }
    for (const std::pair<std::string, VkPushConstantRange>& descriptor_set : push_constants)
    {
        _push_constants.push_back();
    }
    // pipeline layout creation
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = _descriptor_set_layouts.size();
    pipelineLayoutInfo.pSetLayouts = _descriptor_set_layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = _push_constants.size();
    pipelineLayoutInfo.pPushConstantRanges = _push_constants.data();
    if (vkCreatePipelineLayout(gpu->_logical_device, &pipelineLayoutInfo, nullptr, &_pipeline_layout) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create compute pipeline layout!");
    }
    // pipeline creation
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = _pipeline_layout;
    pipelineInfo.stage = 0;
    if (vkCreateComputePipelines(gpu->_logical_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create compute pipeline!");
    }
}

ComputeShader::~ComputeShader()
{

}