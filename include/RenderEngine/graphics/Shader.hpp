#pragma once
#include "GPU.hpp"
#include <RenderEngine/utilities/Macro.hpp>
#include <vector>
#include <string>
#include <fstream>

namespace RenderEngine
{
    class Shader
    {
    public:
        Shader() = delete;
        Shader(const Shader& other);
        Shader(const GPU& gpu, const std::string& file_path);
        Shader(const GPU& gpu, const std::vector<unsigned char>& code);
        ~Shader();
    public:
        const GPU& gpu;
        VkShaderModule _vk_shader;
    public:
        static std::vector<unsigned char> load_binary(const std::string& file_path);
    protected:
        void _set_vk_shader(const std::vector<unsigned char>& code);
    };
}
