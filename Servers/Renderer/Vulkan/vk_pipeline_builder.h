#pragma once

#include "vk_tools.h"

#include <string>
#include <vector>
#include <memory>

#include "spirv_reflect.h"

#include "vk_descriptors.h"

#include <glm/glm.hpp>

struct VertexInputDescription {

	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;

	VkPipelineVertexInputStateCreateFlags flags = 0;
};



struct DescriptorSetLayoutData {
  uint32_t set_number;
  VkDescriptorSetLayoutCreateInfo create_info = {};
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
};

struct DescriptorInfo {
    struct BindingInfo {
        VkShaderStageFlags shader_stage_flags;
        VkDescriptorType descriptor_types;
        uint32_t binding = 0;
    };
    
    std::vector<BindingInfo> binding_info;
    VkDescriptorSet set = VK_NULL_HANDLE;
};

struct ShaderProgram {
    VkPipeline pipeline;
    VkPipelineLayout layout;

    VkViewport viewport;
	VkRect2D scissor;

    std::vector<ShaderPass> passes;

    std::vector<VkPushConstantRange> push_constants;

    std::vector<DescriptorInfo> descriptor_sets;

    DescriptorBuilder descriptor_builder;

    void DestroyProgram();
};

class PipelineBuilder {
public:
	static inline VkPipelineVertexInputStateCreateInfo vertex_input_info;
	static inline VkPipelineInputAssemblyStateCreateInfo input_assembly;
	static inline VkPipelineRasterizationStateCreateInfo rasterizer;
	static inline VkPipelineColorBlendAttachmentState color_blend_attachment;
	static inline VkPipelineMultisampleStateCreateInfo multisampling;
    static inline VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
	static inline VkPipelineLayout pipeline_layout;

    static inline uint32_t stride;

    static inline std::vector<uint32_t> vertex_offsets;

    //descriptor set layout cache object
    static inline DescriptorLayoutCache l_cache;
    //descriptor set allocator object
    static inline DescriptorAllocator d_alloc;

    static void BuildShaderProgram(ShaderProgram& shader_program);

    static void CleanUp();
    
};