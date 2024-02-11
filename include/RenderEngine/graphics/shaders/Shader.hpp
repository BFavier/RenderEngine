#pragma once
#include <RenderEngine/graphics/shaders/ShaderSubpass.hpp>
#include <vector>
#include <memory>

// An exemple of shader wilth multiple subpasses :
// https://github.com/SaschaWillems/Vulkan/blob/master/examples/subpasses/subpasses.cpp
// https://github.com/SaschaWillems/Vulkan/tree/master/shaders/glsl/subpasses

namespace RenderEngine
{
    class GPU;

    class Shader
    // A Shader is a succession of ShaderSubpass
    {
    public:
        enum Type{FLOAT, VEC2, VEC3, VEC4, INT, IVEC2, IVEC3, IVEC4, UINT, UVEC2, UVEC3, UVEC4, DOUBLE, DVEC2, DVEC3, DVEC4};
        static const std::vector<VkFormat> Format;
        static const std::vector<unsigned char> ByteSize;
    public: // This object is non copyable
        Shader() = delete;
        Shader(const Shader& other) = delete;
        const Shader& operator=(const Shader& other) = delete;
    public:
        Shader(const std::shared_ptr<GPU>& gpu,
               const std::vector<std::vector<std::pair<std::string, Type>>>& vertex_buffer_inputs,
               const std::vector<std::vector<std::pair<std::string, Type>>>& fragment_inputs,
               const std::vector<std::vector<std::pair<std::string, Type>>>& fragment_outputs);
        ~Shader();
    protected:
        std::shared_ptr<GPU> gpu;
        VkRenderPass _render_pass = VK_NULL_HANDLE;
        std::vector<VkPipelineLayout> _pipeline_layouts;
        std::vector<std::vector<VkShaderModule>> _modules;
        std::vector<VkPipeline> _pipelines;
    protected:
        void _create_render_pass(const std::vector<std::vector<std::pair<std::string, Type>>>& fragment_inputs,
                                 const std::vector<std::vector<std::pair<std::string, Type>>>& fragment_outputs);
        void _create_pipeline();
        VkShaderModule _code_to_module(const std::vector<unsigned char>& code);
    };
}
