#include <RenderEngine/graphics/shaders/Shader.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <RenderEngine/utilities/Macro.hpp>
using namespace RenderEngine;

Shader::Shader(const GPU* gpu,
               const std::vector<std::pair<std::string, VkVertexInputAttributeDescription>>& vertex_buffers,
               const std::vector<std::pair<std::string, VkFormat>>& input_attachments,
               const std::vector<std::pair<std::string, VkFormat>>& output_attachments,
               const std::vector<std::map<std::string, VkDescriptorSetLayoutBinding>>& descriptor_sets, // for each layout set, descriptor of all bindings (textures, Uniform Buffer Objects, ...)
               const std::map<std::string, VkPushConstantRange>& push_constants, // definition of all push constants.
               const std::map<VkShaderStageFlagBits, std::vector<uint8_t>> shader_stages_bytecode, // the bytecode of the compiled spirv file
               bool depth_test,
               Blending blending
               ) : _gpu(gpu)
{
    _push_constants = push_constants;
    _input_attachments = input_attachments;
    _output_attachments = output_attachments;
    _descriptor_sets = descriptor_sets;
    std::tie(_vk_render_pass, _final_layouts) = _create_render_pass(*gpu, input_attachments, output_attachments, depth_test);
    _descriptor_set_layouts = _create_descriptor_set_layouts(*gpu, descriptor_sets);
    _modules = _create_modules(*gpu, shader_stages_bytecode);
    _vk_pipeline_layout = _create_pipeline_layout(*gpu, push_constants, _descriptor_set_layouts);
    if (_modules.find(VK_SHADER_STAGE_COMPUTE_BIT) != _modules.end())
    {
        _depth_test = false;
        _vk_pipeline_bind_point = VK_PIPELINE_BIND_POINT_COMPUTE;
        _vk_pipeline = _create_compute_pipeline(*gpu, _vk_pipeline_layout, _modules);
    }
    else
    {
        _depth_test = depth_test;
        _vk_pipeline_bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
        _vk_pipeline = _create_graphics_pipeline(*gpu, vertex_buffers, output_attachments, _modules, _vk_pipeline_layout, _vk_render_pass, depth_test, blending);
    }
}

Shader::~Shader()
{
    vkDestroyPipeline(_gpu->_logical_device, _vk_pipeline, nullptr);
    vkDestroyPipelineLayout(_gpu->_logical_device, _vk_pipeline_layout, nullptr);
    for (const VkDescriptorSetLayout& desc : _descriptor_set_layouts)
    {
        vkDestroyDescriptorSetLayout(_gpu->_logical_device, desc, nullptr);
    }
    for (const std::pair<VkShaderStageFlagBits, VkShaderModule>& module : _modules)
    {
        vkDestroyShaderModule(_gpu->_logical_device, module.second, nullptr);
    }
    vkDestroyRenderPass(_gpu->_logical_device, _vk_render_pass, nullptr);
}


std::tuple<VkRenderPass, std::map<std::string, VkImageLayout>> Shader::_create_render_pass(const GPU& gpu,
                                                                                           const std::vector<std::pair<std::string, VkFormat>>& input_attachments,
                                                                                           const std::vector<std::pair<std::string, VkFormat>>& output_attachments,
                                                                                           bool depth_test)
{
    // Creating input/output attachment descriptions
    std::map<std::string, VkImageLayout> output_layouts;
    std::vector<VkAttachmentDescription> attachments;
    {
        // input attachments
        for (const std::pair<std::string, VkFormat>& att : input_attachments)
        {
            VkAttachmentDescription attachment{};
            attachment.format = static_cast<VkFormat>(att.second);
            attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachments.push_back(attachment);
            output_layouts[att.first] = attachment.finalLayout;
        }
        // output (color) attachments
        for (const std::pair<std::string, VkFormat>& att : output_attachments)
        {
            VkAttachmentDescription attachment{};
            attachment.format = static_cast<VkFormat>(att.second);
            attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachments.push_back(attachment);
            output_layouts[att.first] = attachment.finalLayout;
        }
        // Depth attachments
        if (depth_test)
        {
            VkAttachmentDescription attachment{};
            attachment.format = gpu.depth_format().second;
            attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachments.push_back(attachment);
            output_layouts["depth"] = attachment.finalLayout;
        }
    }
    // Creating attachment references
    std::vector<VkAttachmentReference> color_attachment_refs;
    std::vector<VkAttachmentReference> input_attachment_refs;
    std::optional<VkAttachmentReference> depth_buffer_ref;
    {
        // Input attachments
        for (uint32_t i=0; i<input_attachments.size(); i++)
        {
            input_attachment_refs.push_back({i, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
        }
        // Output attachments
        for (uint32_t i=0; i<output_attachments.size(); i++)
        {
            color_attachment_refs.push_back({i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        }
        // Depth attachment
        if (depth_test)
        {
            depth_buffer_ref = {static_cast<uint32_t>(attachments.size() - 1), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
        }
    }
    // Creating subpasses
    VkSubpassDescription subpass{};
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = color_attachment_refs.size();
    subpass.pColorAttachments = color_attachment_refs.data(); // Array of attachments the fragment shader writes to
    subpass.pResolveAttachments = nullptr; // Array of same size as color attachments for multisampling anti-aliasing
    subpass.inputAttachmentCount = input_attachment_refs.size();
    subpass.pInputAttachments = input_attachment_refs.data(); // Array of attachments the fragment shader reads from
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr; // Array of reserved attachment positions
    subpass.pDepthStencilAttachment = (depth_buffer_ref.has_value()) ? &depth_buffer_ref.value() : nullptr;
    // Creating render pass
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 0;
    renderPassInfo.pDependencies = nullptr;
    VkRenderPass render_pass = VK_NULL_HANDLE;
    if (vkCreateRenderPass(gpu._logical_device, &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
    return std::make_tuple(render_pass, output_layouts);
}

std::vector<VkDescriptorSetLayout> Shader::_create_descriptor_set_layouts(const GPU& gpu,
                                                                          const std::vector<std::map<std::string, VkDescriptorSetLayoutBinding>>& descriptors_sets)
{
    // Create descriptor set layouts
    // https://stackoverflow.com/questions/56928041/what-is-the-purpose-of-multiple-setlayoutcounts-of-vulkan-vkpipelinelayoutcreate
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts(descriptors_sets.size());
    for (unsigned int i=0;i< descriptors_sets.size();i++) // for each descriptor set
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings_desc;
        for (const std::pair<std::string, VkDescriptorSetLayoutBinding>& b : descriptors_sets[i])
        {
            bindings_desc.push_back(b.second);
        }
        VkDescriptorSetLayoutCreateInfo desc_set_info{};
        desc_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        desc_set_info.flags = (i == descriptors_sets.size()-1) ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR : 0;
        desc_set_info.bindingCount = bindings_desc.size();
        desc_set_info.pBindings = bindings_desc.data();
        if (vkCreateDescriptorSetLayout(gpu._logical_device, &desc_set_info, nullptr, &descriptor_set_layouts[i]) != VK_SUCCESS)
        {
            THROW_ERROR("failed to create descriptor set layout!");
        }
    }
    return descriptor_set_layouts;
}

VkPipelineLayout Shader::_create_pipeline_layout(const GPU& gpu,
    const std::map<std::string, VkPushConstantRange>& push_constants,
    const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts)
{
    // Creating the pipeline layouts
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    std::vector<VkPushConstantRange> push_constant_ranges;
    for (const std::pair<std::string, VkPushConstantRange>& push_constant : push_constants)
    {
        push_constant_ranges.push_back(push_constant.second);
    }
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = descriptor_set_layouts.size();
    pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
    pipeline_layout_info.pushConstantRangeCount = push_constant_ranges.size();
    pipeline_layout_info.pPushConstantRanges = push_constant_ranges.data();
    if (vkCreatePipelineLayout(gpu._logical_device, &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create pipeline layout!");
    }
    return pipeline_layout;
}

std::map<VkShaderStageFlagBits, VkShaderModule> Shader::_create_modules(const GPU& gpu,
                                                    const std::map<VkShaderStageFlagBits, std::vector<uint8_t>> shader_stages_bytecode)
{
    std::map<VkShaderStageFlagBits, VkShaderModule> modules;
    for (const std::pair<VkShaderStageFlagBits, std::vector<uint8_t>>& pair : shader_stages_bytecode)
    {
        modules[pair.first] = _code_to_module(gpu, pair.second);
    }
    return modules;
}

VkPipeline Shader::_create_graphics_pipeline(const GPU& gpu,
                               const std::vector<std::pair<std::string, VkVertexInputAttributeDescription>>& vertex_buffers,
                               const std::vector<std::pair<std::string, VkFormat>>& output_attachments,
                               const std::map<VkShaderStageFlagBits, VkShaderModule>& modules,
                               const VkPipelineLayout& pipeline_layout,
                               const VkRenderPass& render_pass,
                               bool depth_test,
                               Blending blending)
{
    // Staging shader modules
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    for (const std::pair<VkShaderStageFlagBits, VkShaderModule>& module : modules)
    {
        VkPipelineShaderStageCreateInfo stage_info{};
        stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_info.stage = module.first;
        stage_info.module = module.second;
        stage_info.pName = "main";
        shader_stages.push_back(stage_info);
    }
    // Setting dynamic state
    std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_CULL_MODE};
    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = dynamic_states.size();
    dynamic_state.pDynamicStates = dynamic_states.data();
    // Setting vertex buffer layout
    std::vector<VkVertexInputAttributeDescription> vertex_input_attributes;
    for (const std::pair<std::string, VkVertexInputAttributeDescription>& vb : vertex_buffers)
    {
        vertex_input_attributes.push_back(vb.second);
    }
    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    VkVertexInputBindingDescription input_binding_description = {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &input_binding_description;
    vertex_input_info.vertexAttributeDescriptionCount = vertex_input_attributes.size();
    vertex_input_info.pVertexAttributeDescriptions = vertex_input_attributes.data();
    // Setting input assembly
    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;  // See VkPrimitiveTopology for more primitives, they might require that GPU features be enabled
    input_assembly.primitiveRestartEnable = VK_FALSE;
    // Setting viewport and scisor counts
    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = nullptr;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = nullptr;
    // Setting the rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // See VkPolygonMode for more draw modes, they might require that GPU features be enabled
    rasterizer.lineWidth = 1.0f; // wideLines GPU feature must be enabled for lineWidth > 1.0
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //VK_FRONT_FACE_CLOCKWISE;
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
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = depth_test ? VK_TRUE : VK_FALSE;
    depth_stencil.depthWriteEnable = depth_test ? VK_TRUE : VK_FALSE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    // Setting color blending
    std::vector<VkPipelineColorBlendAttachmentState> color_blending_attachments;
    for (const std::pair<std::string, VkFormat>& att : output_attachments)
    {
        // There is always only one framebuffer to render to
        VkPipelineColorBlendAttachmentState color_blending_attachment{};
        if (blending == Blending::ALPHA)
        {
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
        else if (blending == Blending::OVERWRITE)
        {
            color_blending_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            color_blending_attachment.blendEnable = VK_TRUE;
            color_blending_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blending_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blending_attachment.colorBlendOp = VK_BLEND_OP_ADD;
            color_blending_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blending_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blending_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
            color_blending_attachments.push_back(color_blending_attachment);
        }
        else if (blending == Blending::ADD)
        {
            color_blending_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            color_blending_attachment.blendEnable = VK_TRUE;
            color_blending_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blending_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blending_attachment.colorBlendOp = VK_BLEND_OP_ADD;
            color_blending_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blending_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blending_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
            color_blending_attachments.push_back(color_blending_attachment);
        }
        else
        {
            THROW_ERROR("Unexpected blending type code : "+std::to_string(blending));
        }
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
    // Creating pipeline
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shader_stages.size();
    pipelineInfo.pStages = shader_stages.data();
    pipelineInfo.pVertexInputState = &vertex_input_info;
    pipelineInfo.pInputAssemblyState = &input_assembly;
    pipelineInfo.pViewportState = &viewport_state;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depth_stencil;
    pipelineInfo.pColorBlendState = &color_blending;
    pipelineInfo.pDynamicState = &dynamic_state;
    pipelineInfo.layout = pipeline_layout;
    pipelineInfo.renderPass = render_pass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional
    if (vkCreateGraphicsPipelines(gpu._logical_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create graphics pipeline!");
    }
    return pipeline;
}


VkPipeline Shader::_create_compute_pipeline(const GPU& gpu,
                               const VkPipelineLayout& pipeline_layout,
                               const std::map<VkShaderStageFlagBits, VkShaderModule>& modules)
{
    // compute stage creation
    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = modules.at(VK_SHADER_STAGE_COMPUTE_BIT);
    computeShaderStageInfo.pName = "main";
    // pipeline creation
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = pipeline_layout;
    pipelineInfo.stage = computeShaderStageInfo;
    if (vkCreateComputePipelines(gpu._logical_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create compute pipeline!");
    }
    return pipeline;
}

VkShaderModule Shader::_code_to_module(const GPU& gpu, const std::vector<uint8_t>& code)
{
    VkShaderModule shader_module;
    VkShaderModuleCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = code.size();
    info.pCode = reinterpret_cast<const uint32_t*>(code.data());
    if (vkCreateShaderModule(gpu._logical_device, &info, nullptr, &shader_module) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create shader module");
    }
    return shader_module;
}