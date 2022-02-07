#include "vk_pipeline_builder.h"

#include "logger.h"
#include "vk_device.h"
#include "vk_defaults.h"
#include "vk_swapchain.h"

#include <cstddef>
#include <fstream>
#include <filesystem>



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
    
    if (vkCreateShaderModule(device, &createInfo, nullptr, &out_shader_pass.module) != VK_SUCCESS) {
        return false;
    }
    
    if (index == 0){
        out_shader_pass.stage = VK_SHADER_STAGE_VERTEX_BIT;
    }
    else {
        out_shader_pass.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    return true;
}


void PipelineBuilder::BuildShaderProgram(ShaderProgram& shader_program) {
    auto& device = DeviceManager::GetVkDevice().device;

    VkPipelineLayoutCreateInfo pipeline_layout_info = vkdefaults::PipelineLayoutCreateInfo();

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

    //build the actual pipeline
	//we now use all of the info structs we have been writing into into this one to create the pipeline
	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.pNext = nullptr;

	pipeline_info.stageCount = shader_program.shader_stages.size();
	pipeline_info.pStages = shader_program.shader_stages.data();
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

}
