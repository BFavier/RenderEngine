#pragma once
#include <RenderEngine/graphics/shaders/Shader.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/graphics/Image.hpp>
using namespace RenderEngine;

const std::vector<VkFormat> Shader::Format = {VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R32_SINT, VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32A32_SINT, VK_FORMAT_R32_UINT, VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32A32_UINT, VK_FORMAT_R64_SFLOAT, VK_FORMAT_R64G64_SFLOAT, VK_FORMAT_R64G64B64_SFLOAT, VK_FORMAT_R64G64B64A64_SFLOAT};
const std::vector<unsigned char> Shader::ByteSize = {4, 8, 16, 32, 4, 8, 16, 32, 4, 8, 16, 32, 8, 16, 32, 64};

Shader::Shader(const std::shared_ptr<GPU>& _gpu,
               const std::vector<std::vector<std::pair<std::string, Type>>>& vertex_inputs,
               const std::vector<std::vector<std::pair<std::string, Type>>>& fragment_inputs,
               const std::vector<std::vector<std::pair<std::string, Type>>>& fragment_outputs
               ) : gpu(_gpu)
{
    _create_render_pass(fragment_inputs, fragment_outputs);
    _create_pipeline();
}

Shader::~Shader()
{
    for (VkPipeline& pipeline : _pipelines)
    {
        vkDestroyPipeline(gpu->_logical_device, pipeline, nullptr);
    }
    for (VkPipelineLayout& pipeline : _pipeline_layouts)
    {
        vkDestroyPipelineLayout(gpu->_logical_device, pipeline, nullptr);
    }
    for (std::vector<VkShaderModule>& stage : _modules)
    {
        for (VkShaderModule& module : stage)
        {
            vkDestroyShaderModule(gpu->_logical_device, module, nullptr);
        }
    }
    vkDestroyRenderPass(gpu->_logical_device, _render_pass, nullptr);
}


void Shader::_create_render_pass(const std::vector<std::vector<std::pair<std::string, Type>>>& fragment_inputs,
                                 const std::vector<std::vector<std::pair<std::string, Type>>>& fragment_outputs)
{
    // Creating attachment references
    std::vector<std::vector<VkAttachmentReference>> color_attachment_refs;
    std::vector<std::vector<VkAttachmentReference>> input_attachment_refs;
    std::vector<std::vector<uint32_t>> reserve_attachments;
    std::vector<VkAttachmentReference> depth_buffer_refs;
    for (unsigned int i=0 : )
    {
        // Color atachments
        uint32_t j=0;
        std::vector<VkAttachmentReference> color_refs;
        for (j=j; j<?; j++)
        {
            color_refs.push_back({j, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        }
        color_attachment_refs.push_back(color_refs);
        // Input attachments
        std::vector<VkAttachmentReference> input_refs;
        for (j=j; j<?; j++)
        {
            input_refs.push_back({j, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        }
        input_attachment_refs.push_back(input_refs);
        // Reserved attachments
        std::vector<uint32_t> reserve;
        for (j=j; j<?; j++)
        {
            reserve.push_back(j);
        }
        reserve_attachments.push_back(reserve);
        // Depth attachments
        depth_buffer_refs.push_back({j, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});
    }
    // Creating subpasses
    std::vector<VkSubpassDescription> subpasses;
    for (unsigned int i=0; i<?; i++)
    {
        VkSubpassDescription subpass;
        subpass.flags = 0;
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = color_attachment_refs[i].size();
        subpass.pColorAttachments = color_attachment_refs[i].data(); // Array of attachments the fragment shader writes to
        subpass.pResolveAttachments = nullptr; // Array of same size as color attachments for multisampling anti-aliasing
        subpass.inputAttachmentCount = input_attachment_refs[i].size();
        subpass.pInputAttachments = input_attachment_refs[i].data(); // Array of attachments the fragment shader reads from
        subpass.preserveAttachmentCount = reserve_attachments[i].size();
        subpass.pPreserveAttachments = reserve_attachments[i].data(); // Array of reserved attachment positions
        subpass.pDepthStencilAttachment = &depth_buffer_refs[i];
        subpasses.push_back(subpass);
    }
    // Creating attachment descriptions
    std::vector<VkAttachmentDescription> attachments;
    for ()
    {
        // Color attachments
        for ()
        {
            
            VkAttachmentDescription attachment{};
            attachment.format = static_cast<VkFormat>(Image::Format::RGBA);
            attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            attachments.push_back(attachment);
        }
    }
    // Creating render pass
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = subpasses.size();
    renderPassInfo.pSubpasses = subpasses.data();
    renderPassInfo.dependencyCount = 0;
    renderPassInfo.pDependencies = nullptr;
    if (vkCreateRenderPass(gpu->_logical_device, &renderPassInfo, nullptr, &_render_pass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void Shader::_create_pipeline()
{
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    // Creating the pipeline
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 0; // Optional
    pipeline_layout_info.pSetLayouts = nullptr; // Optional
    pipeline_layout_info.pushConstantRangeCount = 0; // Optional
    pipeline_layout_info.pPushConstantRanges = nullptr; // Optional
    if (vkCreatePipelineLayout(gpu->_logical_device, &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
    // Staging shader modules
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    if (_vertex_shader.has_value())
    {
        VkPipelineShaderStageCreateInfo stage_info{};
        stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        stage_info.module = _vertex_shader.value();
        stage_info.pName = "main";
        shader_stages.push_back(stage_info);
    }
    if (_tessellation_shader.has_value())
    {
        VkPipelineShaderStageCreateInfo stage_info{};
        stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_info.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        stage_info.module = _tessellation_shader.value();
        stage_info.pName = "main";
        shader_stages.push_back(stage_info);
    }
    if (_fragment_shader.has_value())
    {
        VkPipelineShaderStageCreateInfo stage_info{};
        stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stage_info.module = _fragment_shader.value();
        stage_info.pName = "main";
        shader_stages.push_back(stage_info);
    }
    // Setting dynamic state
    std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = dynamic_states.size();
    dynamic_state.pDynamicStates = dynamic_states.data();
    // Setting vertex buffer layout
    std::vector<VkVertexInputBindingDescription> vertex_input_bindings;
    for ()
    {
        VkVertexInputBindingDescription vertex_input_binding;
        vertex_input_binding.binding = 0;
        vertex_input_binding.stride = 0;
        vertex_input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    } 
    std::vector<VkVertexInputAttributeDescription> vertex_input_attributes;
    for ()
    {
        VkVertexInputAttributeDescription vertex_input_attribute{};
        vertex_input_attribute.binding = 0;
        vertex_input_attribute.location = 0;
        vertex_input_attribute.format = VK_FORMAT_R32G32_SFLOAT;
        vertex_input_attribute.offset = 0;
        vertex_input_attributes.push_back(vertex_input_attribute);
    }
    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 0;
    vertex_input_info.pVertexBindingDescriptions = nullptr;
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    vertex_input_info.pVertexAttributeDescriptions = nullptr;
    // Setting input assembly
    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;  // See VkPrimitiveTopology for more primitives, they might require that GPU features be enabled
    input_assembly.primitiveRestartEnable = VK_FALSE;
    // Setting viewport and scisor counts
    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;
    // Setting the rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // See VkPolygonMode for more draw modes, they might require that GPU features be enabled
    rasterizer.lineWidth = 1.0f; // wideLines GPU feature must be enabled for lineWidth > 1.0
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
    // Setting the multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional
    // Setting the depth and stencil buffers
    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.depthTestEnable = false;
    // Setting color blending
    std::vector<VkPipelineColorBlendAttachmentState> color_blending_attachments;
    for ()
    {
        VkPipelineColorBlendAttachmentState color_blending_attachment{};
        color_blending_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blending_attachment.blendEnable = VK_TRUE;
        color_blending_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_blending_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blending_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        color_blending_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blending_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blending_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    }
    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
    color_blending.attachmentCount = color_blending_attachments.size();
    color_blending.pAttachments = color_blending_attachments.data();
    color_blending.blendConstants[0] = 0.0f; // Optional
    color_blending.blendConstants[1] = 0.0f; // Optional
    color_blending.blendConstants[2] = 0.0f; // Optional
    color_blending.blendConstants[3] = 0.0f; // Optional
    // Create graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shader_stages.size();
    pipelineInfo.pStages = shader_stages.data();
    pipelineInfo.pVertexInputState = &vertex_input_info;
    pipelineInfo.pInputAssemblyState = &input_assembly;
    pipelineInfo.pViewportState = &viewport_state;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &color_blending;
    pipelineInfo.pDynamicState = &dynamic_state;
    pipelineInfo.layout = pipeline_layout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional
    if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create graphics pipeline!");
    }
}


VkShaderModule Shader::_code_to_module(const std::vector<unsigned char>& code)
{
    VkShaderModule shader_module;
    VkShaderModuleCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = code.size();
    info.pCode = reinterpret_cast<const uint32_t*>(code.data());
    if (vkCreateShaderModule(gpu->_logical_device, &info, nullptr, &shader_module) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create shader module");
    }
    return shader_module;
}