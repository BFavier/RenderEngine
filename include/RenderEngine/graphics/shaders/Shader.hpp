#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/graphics/shaders/Types.hpp>
#include <map>
#include <vector>
#include <memory>
#include <string>

namespace RenderEngine
{
    class GPU;

    class Shader
    // A Shader is a program with inputs and outputs on the GPU. It is made of a succession of sub-passes, each made of a pipeline of stages
    {
    friend class Canvas; // Canvas need access to pipeline
    public: // This object is non copyable
        Shader() = delete;
        Shader(const Shader& other) = delete;
        const Shader& operator=(const Shader& other) = delete;
        ~Shader();
    protected:
        Shader(const GPU* gpu,
               const std::vector<std::pair<std::string, VkVertexInputAttributeDescription>>& vertex_buffers,
               const std::vector<std::pair<std::string, VkFormat>>& input_attachments,
               const std::vector<std::pair<std::string, VkFormat>>& output_attachments,
               const std::vector<std::map<std::string, VkDescriptorSetLayoutBinding>>& descriptor_sets, // for each layout set, descriptor of all bindings (textures, Uniform Buffer Objects, ...)
               const std::map<std::string, VkPushConstantRange>& push_constants, // definition of all push constants.
               const std::map<VkShaderStageFlagBits, std::vector<uint8_t>> shader_stages_bytecode // the bytecode of the compiled spirv file
               );
    protected:
        const GPU* _gpu = nullptr;
        VkRenderPass _render_pass = VK_NULL_HANDLE;
        VkPipeline _pipeline = VK_NULL_HANDLE;  // pipeline
        VkPipelineLayout _pipeline_layout = VK_NULL_HANDLE; // pipeline layout
        VkPipelineBindPoint _bind_point;
        std::map<VkShaderStageFlagBits, VkShaderModule> _modules;  // shader modules (one for each stage)
        std::vector<std::pair<std::string, VkFormat>> _input_attachments;  // attachment images as inputs (texture, pixel read, ...)
        std::vector<std::pair<std::string, VkFormat>> _output_attachments;  // attachment images as outputs (color, ...)
        std::vector<VkDescriptorSetLayout> _descriptor_set_layouts;  // Descriptor sets (Uniform Buffer Objects, SSBO, ...)
        std::map<std::string, VkPushConstantRange> _push_constants; // The list of (push constant name, description) pairs
    protected:
        static VkRenderPass _create_render_pass(const GPU& gpu,
                                                const std::vector<std::pair<std::string, VkFormat>>& input_attachments,
                                                const std::vector<std::pair<std::string, VkFormat>>& output_attachments);
        static std::vector<VkDescriptorSetLayout> _create_descriptor_set_layouts(const GPU& gpu,
                                                                                 const std::vector<std::map<std::string, VkDescriptorSetLayoutBinding>>& descriptors_sets);
        static VkPipelineLayout _create_pipeline_layout(const GPU& gpu,
                                                    const std::map<std::string, VkPushConstantRange>& push_constants,
                                                    const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts);
        static std::map<VkShaderStageFlagBits, VkShaderModule> _create_modules(const GPU& gpu,
                                                    const std::map<VkShaderStageFlagBits, std::vector<uint8_t>> shader_stages_bytecode);
        static VkPipeline _create_graphics_pipeline(const GPU& gpu,
                               const std::vector<std::pair<std::string, VkVertexInputAttributeDescription>>& vertex_buffer,
                               const std::vector<std::pair<std::string, VkFormat>>& output_attachments,
                               const std::map<VkShaderStageFlagBits, VkShaderModule>& modules,
                               const VkPipelineLayout& pipeline_layout,
                               const VkRenderPass& render_pass);
        static VkPipeline _create_compute_pipeline(const GPU& gpu,
                               const VkPipelineLayout& pipeline_layout,
                               const std::map<VkShaderStageFlagBits, VkShaderModule>& modules);
        static VkShaderModule _code_to_module(const GPU& gpu, const std::vector<uint8_t>& code);
    };
}
