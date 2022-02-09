#pragma once

#include "vk_tools.h"

#include <string>
#include <vector>

#include "spirv_reflect.h"

#include "vk_descriptors.h"

struct DescriptorSetLayoutData {
  uint32_t set_number;
  VkDescriptorSetLayoutCreateInfo create_info;
  std::vector<VkDescriptorSetLayoutBinding> bindings;
};

struct ShaderPass {
    std::string filepath;
    VkShaderModule module;


    SpvReflectShaderModule spv_module;

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

    std::vector<ShaderPass> passes;

    std::vector<VkPushConstantRange> push_constants;

    void DestroyProgram();
};

class PipelineBuilder {
public:
	static inline VkPipelineVertexInputStateCreateInfo vertex_input_info;
	static inline VkPipelineInputAssemblyStateCreateInfo input_assembly;
	static inline VkPipelineRasterizationStateCreateInfo rasterizer;
	static inline VkPipelineColorBlendAttachmentState color_blend_attachment;
	static inline VkPipelineMultisampleStateCreateInfo multisampling;
	static inline VkPipelineLayout pipeline_layout;

    static inline DescriptorLayoutCache l_cache;

    static void BuildShaderProgram(ShaderProgram& shader_program);
    
};