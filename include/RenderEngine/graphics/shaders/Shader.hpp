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
    public:
        enum Blending {OVERWRITE, ALPHA, ADD};
    public: // This object is non copyable
        Shader() = delete;
        Shader(const Shader& other) = delete;
        const Shader& operator=(const Shader& other) = delete;
        ~Shader();
    protected:
        Shader(const GPU* gpu,
               const std::vector<std::pair<std::string, VkVertexInputAttributeDescription>>& vertex_buffers, // description of all the data passed through vertex buffer 
               const std::vector<std::pair<std::string, VkFormat>>& output_attachments, // output attachments (images drawn to)
               const std::vector<std::map<std::string, VkDescriptorSetLayoutBinding>>& descriptor_sets, // for each layout set, descriptor of all bindings (textures, Uniform Buffer Objects, ...)
               const std::map<std::string, VkPushConstantRange>& push_constants, // definition of all push constants.
               bool depth_test,
               Blending blending,
               const std::map<VkShaderStageFlagBits, std::vector<uint8_t>> shader_stages_bytecode // the bytecode of the compiled spirv file
               );
    protected:
        const GPU* _gpu = nullptr;
        bool _depth_test;
        Blending _blending;
        VkRenderPass _vk_render_pass = VK_NULL_HANDLE;
        VkPipeline _vk_pipeline = VK_NULL_HANDLE;  // pipeline
        VkPipelineLayout _vk_pipeline_layout = VK_NULL_HANDLE; // pipeline layout
        VkPipelineBindPoint _vk_pipeline_bind_point;
        std::map<VkShaderStageFlagBits, VkShaderModule> _modules;  // shader modules (one for each stage)
        std::vector<std::pair<std::string, VkFormat>> _output_attachments;  // attachment images as outputs (color, ...)
        std::map<std::string, VkImageLayout> _final_layouts;  // final layouts
        std::vector<std::map<std::string, VkDescriptorSetLayoutBinding>> _descriptor_sets;
        std::vector<VkDescriptorSetLayout> _descriptor_set_layouts;  // Descriptor sets (Uniform Buffer Objects, SSBO, ...)
        std::map<std::string, VkPushConstantRange> _push_constants; // The list of (push constant name, description) pairs
    protected:
        static std::tuple<VkRenderPass, std::map<std::string, VkImageLayout>> _create_render_pass(const GPU& gpu,
                                                                                                  const std::vector<std::pair<std::string, VkFormat>>& output_attachments,
                                                                                                  bool depth_test);
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
                               const VkRenderPass& render_pass,
                               bool depth_test,
                               Blending blending);
        static VkPipeline _create_compute_pipeline(const GPU& gpu,
                               const VkPipelineLayout& pipeline_layout,
                               const std::map<VkShaderStageFlagBits, VkShaderModule>& modules);
        static VkShaderModule _code_to_module(const GPU& gpu, const std::vector<uint8_t>& code);
    };
}
