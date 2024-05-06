#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/graphics/ImageFormat.hpp>
#include <RenderEngine/graphics/shaders/Types.hpp>
#include <RenderEngine/graphics/shaders/Vertex.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <vector>
#include <memory>
#include <string>

// An exemple of shader wilth multiple subpasses :
// https://github.com/SaschaWillems/Vulkan/blob/master/examples/subpasses/subpasses.cpp
// https://github.com/SaschaWillems/Vulkan/tree/master/shaders/glsl/subpasses

namespace RenderEngine
{
    class GPU;

    class ComputeShader
    // A ComputeShader is a program with inputs and outputs on the GPU. It is made of a succession of sub-passes, each made of a pipeline of stages
    {
    public: // This object is non copyable
        ComputeShader() = delete;
        ComputeShader(const ComputeShader& other) = delete;
        const ComputeShader& operator=(const ComputeShader& other) = delete;
    public:
        ComputeShader(const GPU* gpu,
               const std::vector<std::map<std::string, VkDescriptorSetLayoutBinding>>& descriptor_sets, // for each layout set, descriptor of all bindings (textures, Uniform Buffer Objects, ...)
               const std::map<std::string, VkPushConstantRange>& push_constants, // definition of all push constants.
               const std::pair<VkShaderStageFlagBits, std::vector<uint8_t>> stages_bytecode // the bytecode of the compiled spirv file
               );
        ~ComputeShader();
    public:
        VkPipeline _pipeline;  // pipeline for each subpass
        VkPipelineLayout _pipeline_layout; // pipeline layout for each subpass
        std::map<std::string, VkDescriptorType> _bindings; // The list of all unique bindings names and types
        std::map<std::string, ImageFormat> _attachments; // The list of all unique color attachments names and types
        std::map<std::string, VkPushConstantRange> _push_constants; // The list of (push constant name, description) pairs
    protected:
        const GPU* gpu = nullptr;
        VkShaderModule _module; // the module
        std::vector<VkDescriptorSetLayout> _descriptor_set_layouts; // descriptor sets layout
    protected:
        VkShaderModule _code_to_module(const std::vector<unsigned char>& code);
    };
}
