#include <RenderEngine/graphics/Shader.hpp>
using namespace RenderEngine;

Shader::Shader(const Shader& other) : gpu(other.gpu)
{
    _vk_shader = other._vk_shader;
}

Shader::Shader(const GPU& _gpu, const std::string& file_path) : gpu(_gpu)
{
    _set_vk_shader(load_binary(file_path));
}

Shader::Shader(const GPU& _gpu, const std::vector<unsigned char>& code) : gpu(_gpu)
{
    _set_vk_shader(code);
}

std::vector<unsigned char> Shader::load_binary(const std::string& file_path)
{
    std::ifstream file(file_path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        THROW_ERROR("failed to open file")
    }
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<unsigned char> buffer(fileSize);
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    file.close();
    return buffer;
}

void Shader::_set_vk_shader(const std::vector<unsigned char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    if (vkCreateShaderModule(gpu._logical_device, &createInfo, nullptr, &_vk_shader) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create shader module")
    }
}