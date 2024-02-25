#pragma once
#include <RenderEngine/graphics/shaders/Shader.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/graphics/Image.hpp>
using namespace RenderEngine;

Shader::Shader(const GPU* _gpu,
               const std::vector<std::vector<std::vector<VkDescriptorSetLayoutBinding>>>& bindings_sets,
               const std::vector<std::vector<std::pair<VkVertexInputBindingDescription, VkVertexInputAttributeDescription>>>& vertex_inputs,
               const std::vector<std::vector<std::pair<std::string, Type>>>& fragment_inputs,
               const std::vector<std::vector<std::pair<std::string, Type>>>& fragment_outputs,
               const std::vector<std::vector<std::pair<VkShaderStageFlagBits, std::vector<uint8_t>>>> stages_bytecode
               ) : gpu(_gpu)
{
    _create_render_pass(fragment_inputs, fragment_outputs);
    _create_pipelines(bindings_sets, vertex_inputs, stages_bytecode);
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
    for (const std::vector<VkDescriptorSetLayout>& desc_sets : _descriptor_set_layouts)
    {
        for (const VkDescriptorSetLayout& desc : desc_sets)
        {
            vkDestroyDescriptorSetLayout(gpu->_logical_device, desc, nullptr);
        }
    }
    for (std::vector<std::pair<VkShaderStageFlagBits, VkShaderModule>>& stage : _modules)
    {
        for (std::pair<VkShaderStageFlagBits, VkShaderModule>& module : stage)
        {
            vkDestroyShaderModule(gpu->_logical_device, module.second, nullptr);
        }
    }
    vkDestroyRenderPass(gpu->_logical_device, _render_pass, nullptr);
}


void Shader::_create_render_pass(const std::vector<std::vector<std::pair<std::string, Type>>>& fragment_inputs,
                                 const std::vector<std::vector<std::pair<std::string, Type>>>& fragment_outputs)
{
    // Ordering color attachments
    std::map<std::pair<std::string, Type>, unsigned int> attachment_indexes; // The attachment index that is attributed to each unique attachments
    std::map<std::pair<std::string, Type>, std::set<unsigned int>> input_subpasses; // subpass indexes at which the given attachment is used as input
    std::map<std::pair<std::string, Type>, std::set<unsigned int>> output_subpasses;  // subpass indexes at which the given attachment is used as output
    {
        std::set<std::pair<std::string, Type>> set_unique_attachments;
        for (unsigned int i=0;i<fragment_inputs.size();i++)
        {
            for (const std::pair<std::string, Type>& in : fragment_inputs[i])
            {
                set_unique_attachments.insert(in);
                input_subpasses[in].insert(i);
            }
            for (const std::pair<std::string, Type>& out : fragment_outputs[i])
            {
                set_unique_attachments.insert(out);
                output_subpasses[out].insert(i);
            }
        }
        unsigned int index = 0;
        for (const std::pair<std::string, Type>& att : set_unique_attachments)
        {
            attachment_indexes[att] = index;
            _color_attachments.push_back(att);
            index += 1;
        }
    }
    // Creating unique attachment descriptions
    std::vector<VkAttachmentDescription> attachments;
    {
        // Color attachments
        for (const std::pair<std::string, Type>& att : _color_attachments)
        {
            VkAttachmentDescription attachment{};
            attachment.format = static_cast<VkFormat>(att.second);
            attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachment.storeOp = output_subpasses[att].size() == 0 ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.initialLayout = input_subpasses[att].size() == 0 ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachment.finalLayout = output_subpasses[att].size() == 0 ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            attachments.push_back(attachment);
        }
        // Depth attachments
        VkAttachmentDescription attachment{};
        attachment.format = gpu->depth_format().second;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments.push_back(attachment);
    }
    // Creating attachment references and attachment reserve
    std::vector<std::vector<VkAttachmentReference>> color_attachment_refs;
    std::vector<std::vector<VkAttachmentReference>> input_attachment_refs;
    std::vector<std::vector<uint32_t>> reserve_attachments;
    std::vector<VkAttachmentReference> depth_buffer_refs;
    for (unsigned int i=0;i<fragment_inputs.size();i++)
    {
        // Color attachments
        uint32_t j=0;
        std::vector<VkAttachmentReference> color_refs;
        for (const std::pair<std::string, Type>& att : fragment_outputs[i])
        {
            color_refs.push_back({attachment_indexes[att], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        }
        color_attachment_refs.push_back(color_refs);
        // Input attachments
        std::vector<VkAttachmentReference> input_refs;
        for (const std::pair<std::string, Type>& att : fragment_inputs[i])
        {
            input_refs.push_back({attachment_indexes[att], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        }
        input_attachment_refs.push_back(input_refs);
        // Reserved attachments
        std::vector<uint32_t> reserve;
        uint32_t j=0;
        for (const std::pair<std::string, Type>& att : _color_attachments)
        {
            if (std::find(fragment_inputs[i].begin(), fragment_inputs[i].end(), att) == fragment_inputs[i].end()
                && std::find(fragment_outputs[i].begin(), fragment_outputs[i].end(), att) == fragment_outputs[i].end())
            {
                reserve.push_back(j);
            }
            j += 1;
        }
        reserve_attachments.push_back(reserve);
        // Depth attachments
        depth_buffer_refs.push_back({j, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});
    }
    // Creating attachment dependencies
    std::vector<VkSubpassDependency> dependencies; // https://www.reddit.com/r/vulkan/comments/s80reu/subpass_dependencies_what_are_those_and_why_do_i/
    for (unsigned int i=0;i<fragment_inputs.size()-1;i++)
    {
        VkSubpassDependency dependency{}; // A dependency of an attachment between two subpasses
        dependency.srcSubpass = i;
        dependency.dstSubpass = i+1;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies.push_back(dependency);
    }
    // Creating subpasses
    std::vector<VkSubpassDescription> subpasses;
    for (unsigned int i=0;i<fragment_inputs.size();i++)
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
    // Creating render pass
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = subpasses.size();
    renderPassInfo.pSubpasses = subpasses.data();
    renderPassInfo.dependencyCount = dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();
    if (vkCreateRenderPass(gpu->_logical_device, &renderPassInfo, nullptr, &_render_pass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void Shader::_create_pipelines(const std::vector<std::vector<std::vector<VkDescriptorSetLayoutBinding>>>& bindings_sets,
                               const std::vector<std::vector<std::pair<VkVertexInputBindingDescription, VkVertexInputAttributeDescription>>>& vertex_inputs,
                               const std::vector<std::vector<std::pair<VkShaderStageFlagBits, std::vector<uint8_t>>>> stages_bytecode)
{
    // Create descriptor set layouts
    // https://stackoverflow.com/questions/56928041/what-is-the-purpose-of-multiple-setlayoutcounts-of-vulkan-vkpipelinelayoutcreate
    _descriptor_set_layouts.resize(bindings_sets.size()); // description of all descriptor of a given set of bindings : "layout (set = 0, binding = 1) uniform sampler2D Texture"
    for (unsigned int i=0;i<bindings_sets.size();i++)
    {
        _descriptor_set_layouts[i].resize(bindings_sets[i].size());
        for (unsigned int j=0;j<bindings_sets[i].size();j++)
        {
            VkDescriptorSetLayoutCreateInfo desc_set_info{};
            desc_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            desc_set_info.flags = 0;
            desc_set_info.bindingCount = bindings_sets[i][j].size();
            desc_set_info.pBindings = bindings_sets[i][j].data();
            if (vkCreateDescriptorSetLayout(gpu->_logical_device, &desc_set_info, nullptr, &_descriptor_set_layouts[i][j]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }
    }
    // Creating the pipeline layouts
    _pipeline_layouts.resize(stages_bytecode.size());
    for (unsigned int i=0;i<bindings_sets.size();i++)
    {
        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = _descriptor_set_layouts[i].size();
        pipeline_layout_info.pSetLayouts = _descriptor_set_layouts[i].data();
        pipeline_layout_info.pushConstantRangeCount = 0; // Optional
        pipeline_layout_info.pPushConstantRanges = nullptr; // Optional
        if (vkCreatePipelineLayout(gpu->_logical_device, &pipeline_layout_info, nullptr, &_pipeline_layouts[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }
    //Creating pipeline for each subpass
    _modules.resize(stages_bytecode.size());
    _pipelines.resize(stages_bytecode.size());
    for (unsigned int i=0;i<stages_bytecode.size();i++)
    {
        // Creating modules
        for (const std::pair<VkShaderStageFlagBits, std::vector<uint8_t>>& pair : stages_bytecode[i])
        {
            _modules[i].push_back(std::make_pair(pair.first, _code_to_module(pair.second)));
        }
        // Staging shader modules
        std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
        for (unsigned int j=0;j<stages_bytecode[i].size();j++)
        {
            VkPipelineShaderStageCreateInfo stage_info{};
            stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stage_info.stage = _modules[i][j].first;
            stage_info.module = _modules[i][j].second;
            stage_info.pName = "";
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
        std::vector<VkVertexInputAttributeDescription> vertex_input_attributes;
        for (const std::pair<VkVertexInputBindingDescription, VkVertexInputAttributeDescription>& vinput : vertex_inputs[i])
        {
            vertex_input_bindings.push_back(vinput.first);
            vertex_input_attributes.push_back(vinput.second);
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
        for (const std::pair<std::string, Type>& att : _color_attachments)
        {
            // There is always only one framebuffer to render to
            VkPipelineColorBlendAttachmentState color_blending_attachment{};
            color_blending_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            color_blending_attachment.blendEnable = VK_TRUE;
            color_blending_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            color_blending_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            color_blending_attachment.colorBlendOp = VK_BLEND_OP_ADD;
            color_blending_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blending_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            color_blending_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
            color_blending_attachments.push_back(color_blending_attachment);
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
        pipelineInfo.layout = _pipeline_layouts[i];
        pipelineInfo.renderPass = _render_pass;
        pipelineInfo.subpass = i;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional
        if (vkCreateGraphicsPipelines(gpu->_logical_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipelines[i]) != VK_SUCCESS)
        {
            THROW_ERROR("failed to create graphics pipeline!");
        }
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