#pragma once

#include "vk_tools.h"

#include <string>
#include <vector>

struct ShaderPass {
    std::string filepath;
    VkShaderModule module;
    VkShaderStageFlagBits stage;
};

class ShaderProgramBuilder {
public:
    static bool LoadShaderModule(int index, ShaderPass& out_shader_pass);
private: 
    static inline std::array<std::string, 2> shader_stage_extensions = {
        ".vert",
        ".frag"
    };
};

struct ShaderProgram {
    VkPipeline pipeline;
    VkPipelineLayout layout;

    VkViewport viewport;
	VkRect2D scissor;

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
};

class PipelineBuilder {
public:
	VkPipelineVertexInputStateCreateInfo vertex_input_info;
	VkPipelineInputAssemblyStateCreateInfo input_assembly;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineColorBlendAttachmentState color_blend_attachment;
	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineLayout pipeline_layout;

    void BuildShaderProgram(ShaderProgram& shader_program);
    
};