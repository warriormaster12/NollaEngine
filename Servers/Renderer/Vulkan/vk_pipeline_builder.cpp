#include "vk_pipeline_builder.h"

#include "logger.h"
#include "vk_device.h"
#include "vk_defaults.h"
#include "vk_swapchain.h"




#include <cstddef>
#include <fstream>
#include <filesystem>
#include <vector>






bool ShaderProgramBuilder::LoadShaderModule(int index, ShaderPass& out_shader_pass) {

    auto& device = DeviceManager::GetVkDevice().device;

    //open the file. With cursor at the end
	std::ifstream file(out_shader_pass.filepath.c_str(), std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		return false;
	}


    //find what the size of the file is by looking up the location of the cursor
    //because the cursor is at the end, it gives the size directly in bytes
    size_t fileSize = (size_t)file.tellg();

    //spirv expects the buffer to be on uint32, so make sure to reserve an int vector big enough for the entire file
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    //put file cursor at beginning
    file.seekg(0);

    //load the entire file into the buffer
    file.read((char*)buffer.data(), fileSize);

    //now that the file is loaded into the buffer, we can close it
    file.close();


    //create a new shader module, using the buffer we loaded
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;

    //codeSize has to be in bytes, so multiply the ints in the buffer by size of int to know the real size of the buffer
    createInfo.codeSize = buffer.size() * sizeof(uint32_t);
    createInfo.pCode = buffer.data();

    //check that the creation goes well.

    // Generate reflection data for a shader
    SpvReflectResult result = spvReflectCreateShaderModule(fileSize, buffer.data(), &out_shader_pass.spv_module);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    // Enumerate and extract shader's input variables
    uint32_t var_count = 0;
    result = spvReflectEnumerateInputVariables(&out_shader_pass.spv_module, &var_count, NULL);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    SpvReflectInterfaceVariable** input_vars =
    (SpvReflectInterfaceVariable**)malloc(var_count * sizeof(SpvReflectInterfaceVariable*));
    result = spvReflectEnumerateInputVariables(&out_shader_pass.spv_module, &var_count, input_vars);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    if (vkCreateShaderModule(device, &createInfo, nullptr, &out_shader_pass.module) != VK_SUCCESS) {
        return false;
    }
    return true;
}


void ShaderProgram::DestroyProgram() {
    vkDestroyPipeline(DeviceManager::GetVkDevice().device, pipeline, nullptr);
    vkDestroyPipelineLayout(DeviceManager::GetVkDevice().device, layout, nullptr);
}


void PipelineBuilder::BuildShaderProgram(ShaderProgram& shader_program) {

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {};
    
    std::vector<VkDescriptorSetLayout> descriptor_layouts = {};
    for(auto& current_pass: shader_program.passes) {
        uint32_t count = 0;
        SpvReflectResult result = spvReflectEnumerateDescriptorSets(&current_pass.spv_module, &count, NULL);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        std::vector<SpvReflectDescriptorSet*> sets(count);
        result = spvReflectEnumerateDescriptorSets(&current_pass.spv_module, &count, sets.data());
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        std::vector<DescriptorSetLayoutData> set_layouts(sets.size(), DescriptorSetLayoutData{});
        shader_program.descriptor_sets.resize(sets.size());
        for (size_t i_set = 0; i_set < sets.size(); ++i_set) {
            const SpvReflectDescriptorSet& refl_set = *(sets[i_set]);
            DescriptorSetLayoutData& layout = set_layouts[i_set];
            layout.bindings.resize(refl_set.binding_count);
            shader_program.descriptor_sets[i_set].binding_info.resize(refl_set.binding_count);
            for (uint32_t i_binding = 0; i_binding < refl_set.binding_count; ++i_binding) {
                const SpvReflectDescriptorBinding& refl_binding = *(refl_set.bindings[i_binding]);
                VkDescriptorSetLayoutBinding& layout_binding = layout.bindings[i_binding];
                layout_binding.binding = refl_binding.binding;
                layout_binding.descriptorType = static_cast<VkDescriptorType>(refl_binding.descriptor_type);
                layout_binding.descriptorCount = 1;
                for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim) {
                    layout_binding.descriptorCount *= refl_binding.array.dims[i_dim];
                }
                layout_binding.stageFlags = static_cast<VkShaderStageFlagBits>(current_pass.spv_module.shader_stage);
                shader_program.descriptor_sets[i_set].binding_info[i_binding].shader_stage_flags = layout_binding.stageFlags;
                shader_program.descriptor_sets[i_set].binding_info[i_binding].descriptor_types = layout_binding.descriptorType;
                shader_program.descriptor_sets[i_set].binding_info[i_binding].binding = i_binding;
            }
            layout.set_number = refl_set.set;
            layout.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layout.create_info.bindingCount = refl_set.binding_count;
            layout.create_info.pBindings = layout.bindings.data();
            
            descriptor_layouts.push_back(l_cache.CreateDescriptorLayout(layout.create_info));
        }
    }
    

    for(auto& current_pass: shader_program.passes){
        shader_stages.push_back(vkdefaults::PipelineShaderStageCreateInfo((VkShaderStageFlagBits)current_pass.spv_module.shader_stage, current_pass.module));
        VkPushConstantRange push_constant = {};
        
        if(current_pass.spv_module.push_constant_blocks != nullptr)
        {
            auto& push_constant_blocks = current_pass.spv_module.push_constant_blocks;

            push_constant.offset = push_constant_blocks->offset;
            push_constant.size = push_constant_blocks->size;
            push_constant.stageFlags = (VkShaderStageFlagBits)current_pass.spv_module.shader_stage;

            shader_program.push_constants.push_back(push_constant);
        }
        
    }

    auto& device = DeviceManager::GetVkDevice().device;

    VkPipelineLayoutCreateInfo pipeline_layout_info = vkdefaults::PipelineLayoutCreateInfo();
    pipeline_layout_info.pPushConstantRanges = shader_program.push_constants.data();
    pipeline_layout_info.pushConstantRangeCount = shader_program.push_constants.size();
    pipeline_layout_info.pSetLayouts = descriptor_layouts.data();
    pipeline_layout_info.setLayoutCount = descriptor_layouts.size();

	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &shader_program.layout));

    //make viewport state from our stored viewport and scissor.
    //at the moment we won't support multiple viewports or scissors
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;

    viewportState.viewportCount = 1;
    viewportState.pViewports = &shader_program.viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &shader_program.scissor;

    //setup dummy color blending. We aren't using transparent objects yet
    //the blending is just "no blend", but we do write to the color attachment
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext = nullptr;

    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &color_blend_attachment;

    std::vector <VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR}; 
    VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
    dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_info.dynamicStateCount = dynamic_states.size();
    dynamic_state_info.pDynamicStates = dynamic_states.data();
        

    //build the actual pipeline
	//we now use all of the info structs we have been writing into into this one to create the pipeline
	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.pNext = nullptr;

	pipeline_info.stageCount = shader_stages.size();
	pipeline_info.pStages = shader_stages.data();
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewportState;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pColorBlendState = &colorBlending;
	pipeline_info.layout = shader_program.layout;
	pipeline_info.renderPass = VK_NULL_HANDLE;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.pDynamicState = &dynamic_state_info;

    // New create info to define color, depth and stencil attachments at pipeline create time
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info = {};
    pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    pipeline_rendering_create_info.colorAttachmentCount = 1;
    pipeline_rendering_create_info.pColorAttachmentFormats = &SwapchainManager::GetVkSwapchain().swapchain_image_format;
    pipeline_rendering_create_info.depthAttachmentFormat = SwapchainManager::GetVkSwapchain().swapchain_depth_format;
    pipeline_rendering_create_info.stencilAttachmentFormat =  SwapchainManager::GetVkSwapchain().swapchain_depth_format;
    pipeline_rendering_create_info.viewMask = 0;

    pipeline_info.pNext = &pipeline_rendering_create_info;


    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &shader_program.pipeline) != VK_SUCCESS)
    {
        ENGINE_CORE_ERROR("failed to create graphics pipeline");
    }

    for(auto& current_pass : shader_program.passes) {
        // Destroy the reflection data when no longer required.
        spvReflectDestroyShaderModule(&current_pass.spv_module);
    }
    

    for(auto& stage : shader_stages)
    {
        vkDestroyShaderModule(DeviceManager::GetVkDevice().device, stage.module, nullptr);
    }

}

void PipelineBuilder::CleanUp() {
    l_cache.CleanUp();
    d_alloc.CleanUp();
}
