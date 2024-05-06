#include <RenderEngine/graphics/shaders/Shader.hpp>
#include <RenderEngine/graphics/shaders/ComputeShader.hpp>
#include <RenderEngine/graphics/GPU.hpp>
using namespace RenderEngine;

ComputeShader::ComputeShader(const GPU* gpu,
               const std::vector<std::map<std::string, VkDescriptorSetLayoutBinding>>& descriptor_sets, // for each layout set, descriptor of all bindings (textures, Uniform Buffer Objects, ...)
               const std::map<std::string, VkPushConstantRange>& push_constants, // definition of all push constants.
               const std::vector<uint8_t> bytecode // the bytecode of the compiled spirv file
               )
{
    // create descriptor set layouts
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
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
        descriptor_set_layouts.push_back(descriptor_set_layout);
    }
    // create push constant ranges
    std::vector<VkPushConstantRange> push_constant_ranges;
    for (const std::pair<std::string, VkPushConstantRange>& descriptor_set : push_constants)
    {
        _push_constants[descriptor_set.first] = descriptor_set.second;
        push_constant_ranges.push_back(descriptor_set.second);
    }
    // pipeline layout creation
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = descriptor_set_layouts.size();
    pipelineLayoutInfo.pSetLayouts = descriptor_set_layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = push_constant_ranges.size();
    pipelineLayoutInfo.pPushConstantRanges = push_constant_ranges.data();
    if (vkCreatePipelineLayout(gpu->_logical_device, &pipelineLayoutInfo, nullptr, &_pipeline_layout) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create compute pipeline layout!");
    }
    // compute stage creation
    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = Shader::code_to_module(*gpu, bytecode);
    computeShaderStageInfo.pName = "main";
    // pipeline creation
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = _pipeline_layout;
    pipelineInfo.stage = computeShaderStageInfo;
    if (vkCreateComputePipelines(gpu->_logical_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create compute pipeline!");
    }
}

ComputeShader::~ComputeShader()
{

}