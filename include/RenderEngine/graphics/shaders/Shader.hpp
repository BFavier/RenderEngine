#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/graphics/shaders/Type.hpp>
#include <RenderEngine/graphics/shaders/Vertex.hpp>
#include <vector>
#include <memory>
#include <string>

// An exemple of shader wilth multiple subpasses :
// https://github.com/SaschaWillems/Vulkan/blob/master/examples/subpasses/subpasses.cpp
// https://github.com/SaschaWillems/Vulkan/tree/master/shaders/glsl/subpasses

namespace RenderEngine
{
    class GPU;

    class Shader
    // A Shader is a program with inputs and outputs on the GPU. It is made of a succession of sub-passes, each made of a pipeline of stages
    {
    public: // This object is non copyable
        Shader() = delete;
        Shader(const Shader& other) = delete;
        const Shader& operator=(const Shader& other) = delete;
    public:
        Shader(const GPU* gpu,
               const std::vector<std::vector<std::pair<std::string, VkVertexInputBindingDescription>>>& vertex_buffer_bindings, // for each subpass, for each binding, the name and bytes-size of the vertex buffer
               const std::vector<std::vector<std::pair<std::string, VkVertexInputAttributeDescription>>>& vertex_buffer_attributes, // for each subpass, for each vertex input, it's description
               const std::vector<std::pair<std::string, Type>>& attachments,  // Description of all attachments (VkImage objects) that are written to in fragment shader stage (excluding depth buffer)
               const std::vector<std::vector<std::string>>& input_attachments, // for each subpass, the description of all input attachments (outputs from another subpass)
               const std::vector<std::vector<std::string>>& output_attachments, // for each subpass the description of all output attachments (VkImage objects that are written to, excepted depth buffer)
               const std::vector<std::vector<std::vector<std::pair<std::string, VkDescriptorSetLayoutBinding>>>>& descriptor_sets, // for each subpass, for each layout set, descriptor of all bindings (textures, Uniform Buffer Objects, ...)
               const std::vector<std::vector<std::pair<VkShaderStageFlagBits, std::vector<uint8_t>>>> stages_bytecode // for each subpass, for each shader stage, the bytecode of the compiled spirv file
               );
        ~Shader();
    protected:
        const GPU* gpu = nullptr;
        VkRenderPass _render_pass = VK_NULL_HANDLE;
        std::vector<std::vector<std::pair<std::string, VkDescriptorType>>> _bindings; // For each subpass, the list of all unique bindings names and types
        std::vector<std::pair<std::string, Type>> _attachments; // The list of all unique color attachments names and types
        std::vector<std::vector<std::pair<VkShaderStageFlagBits, VkShaderModule>>> _modules; // for each subpass, one or more stage modules
        std::vector<std::vector<VkDescriptorSetLayout>> _descriptor_set_layouts; // for each subpass, one or more descriptor set layout
        std::vector<VkPipelineLayout> _pipeline_layouts; // pipeline layout for each subpass
        std::vector<VkPipeline> _pipelines;  // pipeline for each subpass
    protected:
        void _create_render_pass(const std::vector<std::vector<std::string>>& input_attachments,
                                 const std::vector<std::vector<std::string>>& output_attachments);
        void _create_pipelines(const std::vector<std::vector<std::pair<std::string, VkVertexInputBindingDescription>>>& vertex_buffer_bindings,
                               const std::vector<std::vector<std::pair<std::string, VkVertexInputAttributeDescription>>>& vertex_buffer_attributes,
                               const std::vector<std::vector<std::vector<std::pair<std::string, VkDescriptorSetLayoutBinding>>>>& descriptors_sets,
                               const std::vector<std::vector<std::pair<VkShaderStageFlagBits, std::vector<uint8_t>>>> stages_bytecode);
        VkShaderModule _code_to_module(const std::vector<unsigned char>& code);
    };
}
