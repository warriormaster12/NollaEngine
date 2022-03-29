#include "vk_pipeline_builder.h"

#include "logger.h"
#include "spirv_reflect.h"
#include "vk_device.h"
#include "vk_defaults.h"
#include "vk_swapchain.h"




#include <algorithm>
#include <cstddef>
#include <fstream>
#include <filesystem>
#include <iterator>
#include <unordered_map>
#include <vector>



VertexInputDescription GetVertexDescription(std::vector<SpvReflectInterfaceVariable*> input_vars,uint32_t stride, std::vector<uint32_t> offsets)
{
	VertexInputDescription description;

	//we will have just 1 vertex buffer binding, with a per-vertex rate
	VkVertexInputBindingDescription mainBinding = {};
	mainBinding.binding = 0;
	mainBinding.stride = stride;
	mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	description.bindings.push_back(mainBinding);

    std::sort(input_vars.begin(), input_vars.end(),[](SpvReflectInterfaceVariable* a, SpvReflectInterfaceVariable* b) {
        return a->location < b->location;
    });
	for(int i=0; i < input_vars.size(); i++) {
        VkVertexInputAttributeDescription attribute = {};
        attribute.binding = 0;
        attribute.location = input_vars[i]->location;
        attribute.format = (VkFormat)input_vars[i]->format;
        attribute.offset = offsets[i];
        description.attributes.push_back(attribute);
    }

    return description;
}
	



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
    //TODO: make spirv-reflection code into it's own function
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {};
    std::vector<int> set_numbers;
    std::vector<DescriptorSetLayoutData> descriptor_layouts = {};
    for(auto& current_pass: shader_program.passes) {
        uint32_t count = 0;
        SpvReflectResult result = spvReflectEnumerateDescriptorSets(&current_pass.spv_module, &count, NULL);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);
        std::vector<SpvReflectDescriptorSet*> sets(count);
        result = spvReflectEnumerateDescriptorSets(&current_pass.spv_module, &count, sets.data());
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        std::vector<DescriptorSetLayoutData> set_layouts(sets.size(), DescriptorSetLayoutData{});
        for (size_t i_set = 0; i_set < sets.size(); ++i_set) {
            const SpvReflectDescriptorSet& refl_set = *(sets[i_set]);
            DescriptorSetLayoutData& layout = set_layouts[i_set];
            layout.bindings.resize(refl_set.binding_count);
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
            }

            set_numbers.push_back(refl_set.set);
            layout.set_number = refl_set.set;
            layout.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layout.create_info.bindingCount = refl_set.binding_count;
            layout.create_info.pBindings = layout.bindings.data();
            
            descriptor_layouts.push_back(layout);
        }
    }
    
    std::sort(set_numbers.begin(), set_numbers.end());
    std::vector<int>::iterator iterator;
    iterator = std::unique(set_numbers.begin(), set_numbers.end());

    set_numbers.resize(std::distance(set_numbers.begin(), iterator));

    std::vector<DescriptorSetLayoutData> merged_layouts;
    merged_layouts.resize(set_numbers.size());
	std::vector<VkDescriptorSetLayout> layouts;
	for (int i = 0; i < merged_layouts.size(); i++) {
		DescriptorSetLayoutData &ly = merged_layouts[i];

		ly.set_number = i;

		ly.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

		std::unordered_map<int,VkDescriptorSetLayoutBinding> binds;
		for (auto& s : descriptor_layouts) {
			if (s.set_number == i) {
				for (auto& b : s.bindings)
				{
					auto it = binds.find(b.binding);
					if (it == binds.end())
					{
						binds[b.binding] = b;
						//ly.bindings.push_back(b);
					}
					else {
						//merge flags
						binds[b.binding].stageFlags |= b.stageFlags;
					}
					
				}
			}
		}
		for (auto [k, v] : binds)
		{
			ly.bindings.push_back(v);
		}
		//sort the bindings, for hash purposes
		std::sort(ly.bindings.begin(), ly.bindings.end(), [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b) {			
			return a.binding < b.binding;
		});


		ly.create_info.bindingCount = (uint32_t)ly.bindings.size();
		ly.create_info.pBindings = ly.bindings.data();
		ly.create_info.flags = 0;
		ly.create_info.pNext = 0;
		

		layouts.push_back(l_cache.CreateDescriptorLayout(ly.create_info));
	}

    for(auto& layout : merged_layouts) {
        DescriptorInfo descriptor_info;
        descriptor_info.binding_info.resize(layout.bindings.size());
        for(int i = 0; i < layout.bindings.size(); i++) { 
            descriptor_info.binding_info[i].shader_stage_flags = layout.bindings[i].stageFlags;
            descriptor_info.binding_info[i].descriptor_types = layout.bindings[i].descriptorType;
            descriptor_info.binding_info[i].binding = layout.bindings[i].binding;
        }

        shader_program.descriptor_sets.push_back(descriptor_info);
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
    pipeline_layout_info.pSetLayouts = layouts.data();
    pipeline_layout_info.setLayoutCount = layouts.size();

	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &shader_program.layout));

    VertexInputDescription vertex_description = {};
    for(auto& current_pass: shader_program.passes) {
        if (current_pass.spv_module.shader_stage == (VkShaderStageFlags)VK_SHADER_STAGE_VERTEX_BIT) {
            // Enumerate and extract shader's input variables
            uint32_t var_count = 0;
            SpvReflectResult result = spvReflectEnumerateInputVariables(&current_pass.spv_module, &var_count, NULL);
            assert(result == SPV_REFLECT_RESULT_SUCCESS);
            std::vector<SpvReflectInterfaceVariable*> input_vars(var_count);
            result = spvReflectEnumerateInputVariables(&current_pass.spv_module, &var_count, input_vars.data());
            assert(result == SPV_REFLECT_RESULT_SUCCESS);

            vertex_description = GetVertexDescription(input_vars, stride,vertex_offsets);
        }
    }
    
    vertex_input_info.pVertexAttributeDescriptions = vertex_description.attributes.data();
    vertex_input_info.vertexAttributeDescriptionCount = vertex_description.attributes.size();
    vertex_input_info.pVertexBindingDescriptions = vertex_description.bindings.data(); 
    vertex_input_info.vertexBindingDescriptionCount = vertex_description.bindings.size();

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
    pipeline_info.pDepthStencilState = &depth_stencil;
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
