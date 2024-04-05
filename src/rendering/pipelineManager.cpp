#include "rendering/pipelineManager.hpp"

#include "rendering/rendering.hpp"
#include "rendering/shaderManager.hpp"
#include "rendering/resources/vertex.hpp"


#include <iostream>

pipeline_system::pipeline_system(rendering_system* core) :
    _core(core)
{
    ;
}

void pipeline_system::init(std::unique_ptr<DescriptorLayoutCache>& _descriptorLayoutCache, std::unique_ptr<DescriptorAllocator>& _descriptorAllocator)
{
    
}

void pipeline_system::createPipeline(std::string shaderProgramName, VkDescriptorSetLayout descriptorSetLayout)
{

    shader_system& shaderManager = _core->getShaderSystem();
    auto shaderProgram = shaderManager.getShaderProgram(shaderProgramName);

    createShaderStagesInfo(shaderProgram);

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = _shaderStages.size();
    pipelineInfo.pStages = _shaderStages.data();
    pipelineInfo.pVertexInputState = &createVertexInputInfo();
    pipelineInfo.pInputAssemblyState = &createInputAssemblyInfo();
    pipelineInfo.pViewportState = &createViewportStateInfo();
    pipelineInfo.pRasterizationState = &createRasterizerInfo();
    pipelineInfo.pMultisampleState = &createMultisamplingInfo();
    pipelineInfo.pDepthStencilState = &createDepthStencilInfo(); // Optional
    pipelineInfo.pColorBlendState = &createColorBlendingInfo();
    pipelineInfo.pDynamicState = &createDynamicStateInfo(); // Optional
    pipelineInfo.layout = createPipelineLayout(descriptorSetLayout);
    pipelineInfo.renderPass = _core->getRenderPass();
    pipelineInfo.subpass = 0;

    shaderPipeline shaderPipeline;

    VkResult result = vkCreateGraphicsPipelines(_core->getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &shaderPipeline.pipeline);
    shaderPipeline.layout = _pipelineLayout;

    if(result != VK_SUCCESS)
    {
        std::cerr << "Failed to create pipeline" << std::endl;
    }
    else
    {
        _pipelines[shaderProgramName] = shaderPipeline;
    }

    
}

void pipeline_system::cleanup()
{
    for (auto& pipeline : _pipelines)
    {
        vkDestroyPipeline(_core->getLogicalDevice(), pipeline.second.pipeline, nullptr);
    }

    vkDestroyPipelineLayout(_core->getLogicalDevice(), _pipelineLayout, nullptr);
}

shaderPipeline& pipeline_system::getPipeline(std::string name)
{
    return _pipelines[name];
}

std::vector<VkPipelineShaderStageCreateInfo> pipeline_system::createShaderStagesInfo(const shaderProgram& program)
{
    shader_system& shaderManager = _core->getShaderSystem();

    _shaderStages.clear();

    for (auto& shader : program.shaders)
    {
        if (shader.name.empty())
        {
            continue;
        }

        VkPipelineShaderStageCreateInfo shaderStageInfo = {};
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = shaderManager.getVkShaderStageFlagBits(shader.type);
        shaderStageInfo.module = shader.VKmodule;
        shaderStageInfo.pName = "main"; // entry point function for the shader

        _shaderStages.push_back(shaderStageInfo);
    }

    return _shaderStages;
}

VkPipelineVertexInputStateCreateInfo pipeline_system::createVertexInputInfo()
{
    // VkPipelineVertexInputStateCreateInfo temp{};

    // temp.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    // temp.vertexBindingDescriptionCount = 1;
    // temp.pVertexBindingDescriptions = &Vertex::getBindingDescription();
    // temp.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::getAttributeDescriptions().size());;
    // temp.pVertexAttributeDescriptions = Vertex::getAttributeDescriptions().data();


    _bindingDescription = Vertex::getBindingDescription();
    _attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo temp{};
    temp.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    temp.vertexBindingDescriptionCount = 1;
    temp.pVertexBindingDescriptions = &_bindingDescription; 
    temp.vertexAttributeDescriptionCount = static_cast<uint32_t>(_attributeDescriptions.size());
    temp.pVertexAttributeDescriptions = _attributeDescriptions.data(); 

    _vertexInputInfo = temp;

    return _vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo pipeline_system::createInputAssemblyInfo()
{
    VkPipelineInputAssemblyStateCreateInfo temp{};

    temp.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    temp.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    temp.primitiveRestartEnable = VK_FALSE;

    _inputAssembly = temp;

    return _inputAssembly;
}

VkPipelineDynamicStateCreateInfo pipeline_system::createDynamicStateInfo()
{
    _dynamicStates.clear();
    _dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo temp{};

    temp.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    temp.dynamicStateCount = static_cast<uint32_t>(_dynamicStates.size());;
    temp.pDynamicStates = _dynamicStates.data();

    _dynamicState = temp;

    return _dynamicState;
}

VkPipelineViewportStateCreateInfo pipeline_system::createViewportStateInfo()
{
    _viewportState = {};

    _viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    _viewportState.viewportCount = 1;
    _viewportState.pViewports = nullptr;
    _viewportState.scissorCount = 1;
    _viewportState.pScissors = nullptr;
    _viewportState.pNext = nullptr;

    return _viewportState;
}

VkPipelineRasterizationStateCreateInfo pipeline_system::createRasterizerInfo()
{
    _rasterizer = {};

    _rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    _rasterizer.depthClampEnable = VK_FALSE;
    _rasterizer.rasterizerDiscardEnable = VK_FALSE;
    _rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    _rasterizer.lineWidth = 1.0f;
    _rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    _rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    _rasterizer.depthBiasEnable = VK_FALSE;
    _rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    _rasterizer.depthBiasClamp = 0.0f; // Optional
    _rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    return _rasterizer;
}

VkPipelineMultisampleStateCreateInfo pipeline_system::createMultisamplingInfo()
{
    _multisampling = {};

    _multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    _multisampling.sampleShadingEnable = VK_FALSE;
    _multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    _multisampling.minSampleShading = 1.0f; // Optional
    _multisampling.pSampleMask = nullptr; // Optional
    _multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    _multisampling.alphaToOneEnable = VK_FALSE; // Optional

    return _multisampling;
}

VkPipelineDepthStencilStateCreateInfo pipeline_system::createDepthStencilInfo()
{
    _depthStencil = {};

    _depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    _depthStencil.depthTestEnable = VK_TRUE;
    _depthStencil.depthWriteEnable = VK_TRUE;
    _depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    _depthStencil.depthBoundsTestEnable = VK_FALSE;
    _depthStencil.minDepthBounds = 0.0f; // Optional
    _depthStencil.maxDepthBounds = 1.0f; // Optional
    _depthStencil.stencilTestEnable = VK_FALSE;
    _depthStencil.front = {}; // Optional
    _depthStencil.back = {}; // Optional

    return _depthStencil;
}

VkPipelineColorBlendStateCreateInfo pipeline_system::createColorBlendingInfo()
{
    _colorBlendAttachment = {};

    _colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    _colorBlendAttachment.blendEnable = VK_FALSE;
    _colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    _colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    _colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    _colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    _colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    _colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    _colorBlending = {};

    _colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    _colorBlending.logicOpEnable = VK_FALSE;
    _colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    _colorBlending.attachmentCount = 1;
    _colorBlending.pAttachments = &_colorBlendAttachment;
    _colorBlending.blendConstants[0] = 0.0f; // Optional
    _colorBlending.blendConstants[1] = 0.0f; // Optional
    _colorBlending.blendConstants[2] = 0.0f; // Optional
    _colorBlending.blendConstants[3] = 0.0f; // Optional

    return _colorBlending;
}

VkPipelineLayout pipeline_system::createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout)
{
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if(vkCreatePipelineLayout(_core->getLogicalDevice(), &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
    {
        std::cerr << "Failed to create pipeline layout" << std::endl;
    }

    return _pipelineLayout;
}

