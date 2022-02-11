#include "vk_context.h"

#include "logger.h"
#include "vk_descriptors.h"
#include "vk_device.h"
#include "vk_swapchain.h"
#include "vk_commands_&_sync.h"

#include "vk_tools.h"
#include "vk_defaults.h"
#include "vk_pipeline_builder.h"

#include "window.h"
#include <vector>
#include "utils.h"




struct ShaderProgramInfo {
    ShaderProgram program;

    std::unordered_map<int,std::vector<vktools::AllocatedBuffer>> alloc_buffers = {};

};


static std::unordered_map<std::string,ShaderProgramInfo> shader_program;


void VkContext::InitContext() {
    DeviceManager::Init();
    SwapchainManager::Init();
    CommandbufferManager::Init();

    ENGINE_CORE_INFO("vulkan context created");
}

void VkContext::CreatePipeline(const std::string& pipeline_name, std::vector<std::string> filepaths) {
    if(utils::FindUnorderedMap(pipeline_name, shader_program) == nullptr) {
        shader_program[pipeline_name];
        auto& program = utils::FindUnorderedMap(pipeline_name, shader_program)->program;
        for (int i = 0; i < filepaths.size(); i++)
        {
            ShaderPass shader_pass;
            shader_pass.filepath = filepaths[i];
            if(!ShaderProgramBuilder::LoadShaderModule(i, shader_pass)) {
                ENGINE_CORE_ERROR("failed to load shader program: {0}", shader_pass.filepath);
            }
            else {
                ENGINE_CORE_INFO("succefully loaded shader program: {0}", shader_pass.filepath);
            }
            program.passes.push_back(shader_pass);
        }

        //build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
        PipelineBuilder pipeline_builder;

        //vertex input controls how to read vertices from vertex buffers. We aren't using it yet
        PipelineBuilder::vertex_input_info = vkdefaults::VertexInputStateCreateInfo();

        //input assembly is the configuration for drawing triangle lists, strips, or individual points.
        //we are just going to draw triangle list
        PipelineBuilder::input_assembly = vkdefaults::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        //configure the rasterizer to draw filled triangles
        PipelineBuilder::rasterizer = vkdefaults::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

        //we don't use multisampling, so just run the default one
        PipelineBuilder::multisampling = vkdefaults::MultisamplingStateCreateInfo();

        //a single blend attachment with no blending and writing to RGBA
        PipelineBuilder::color_blend_attachment = vkdefaults::ColorBlendAttachmentState();

        //finally build the pipeline
        PipelineBuilder::BuildShaderProgram(program); 
    }   
}

void VkContext::CreateBuffer(const std::string& pipeline_name, int set_index, size_t alloc_size, int usage) {
    if((VkBufferUsageFlags)usage == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT || VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
    {
        auto& program = *utils::FindUnorderedMap(pipeline_name, shader_program);
        auto alloc_buffer = vktools::CreateBuffer(alloc_size, (VkBufferUsageFlags)usage, VMA_MEMORY_USAGE_CPU_TO_GPU);
        VkDescriptorBufferInfo info = {};
        info.buffer = alloc_buffer.buffer;
        info.range = alloc_size;

        
        if(utils::FindUnorderedMap(set_index, program.alloc_buffers) == nullptr){
            program.alloc_buffers[set_index];
        }
        auto& buffers = *utils::FindUnorderedMap(set_index, program.alloc_buffers);
        buffers.push_back(alloc_buffer);
        buffers.back().buffer_info = info;
    }
    //TODO: Vertex and Index buffer support
}

void VkContext::BuildDescriptors(const std::string& pipeline_name) {
    auto& program = *utils::FindUnorderedMap(pipeline_name, shader_program);
    int set_index = 0;
    for(auto& current_set : program.program.descriptor_sets){
        program.program.descriptor_builder = DescriptorBuilder::Begin(PipelineBuilder::l_cache, PipelineBuilder::d_alloc);
        for (auto& current_binding : current_set.binding_info){
            auto& buffers = *utils::FindUnorderedMap(set_index, program.alloc_buffers);
            program.program.descriptor_builder.BindBuffer(current_binding.binding, buffers[current_binding.binding].buffer_info, current_binding.descriptor_types, current_binding.shader_stage_flags);
        }
        program.program.descriptor_builder.Build(current_set.set);

        set_index ++;
    }
}

void VkContext::PrepareFrame() {
    auto& current_frame = CommandbufferManager::GetCurrentFrame(frame_number);
    auto& device = DeviceManager::GetVkDevice().device;

    //wait until the gpu has finished rendering the last frame. Timeout of 1 second
    VK_CHECK_RESULT(vkWaitForFences(device, 1, &current_frame.render_fence, true, 1000000000));
    VK_CHECK_RESULT(vkResetFences(device, 1, &current_frame.render_fence));

    //now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
	VK_CHECK_RESULT(vkResetCommandBuffer(current_frame.main_command_buffer, 0));

    VkResult draw_result = vkAcquireNextImageKHR(
        device, 
        SwapchainManager::GetVkSwapchain().swapchain,
        1000000000,
        current_frame.present_semaphore, 
        nullptr,
        &image_index
    );

    if (draw_result == VK_ERROR_OUT_OF_DATE_KHR) {
		RecreateSwapchain();
        return;
	} else if (draw_result != VK_SUCCESS && draw_result != VK_SUBOPTIMAL_KHR) {
		ENGINE_CORE_ERROR("failed to acquire swap chain image!");
		abort();
	}

    //begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmd_begin_info = vkdefaults::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK_RESULT(vkBeginCommandBuffer(current_frame.main_command_buffer, &cmd_begin_info));
}

void VkContext::BeginNewRenderLayer(std::array<float, 4> color, float depth) {
    auto& current_frame = CommandbufferManager::GetCurrentFrame(frame_number);
    auto& device = DeviceManager::GetVkDevice().device;
    vktools::InsertImageMemoryBarrier(
        current_frame.main_command_buffer,
        SwapchainManager::GetVkSwapchain().swapchain_images[image_index],
        0,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
    

    vktools::InsertImageMemoryBarrier(
        current_frame.main_command_buffer,
        SwapchainManager::GetVkSwapchain().depth_image.image,
        0,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });

    // New structures are used to define the attachments used in dynamic rendering
    VkRenderingAttachmentInfoKHR colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    colorAttachment.imageView = SwapchainManager::GetVkSwapchain().swapchain_image_views[image_index];
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { color[0],color[1],color[2],color[3] };

    // A single depth stencil attachment info can be used, but they can also be specified separately.
    // When both are specified separately, the only requirement is that the image view is identical.			
    VkRenderingAttachmentInfoKHR depthStencilAttachment{};
    depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    depthStencilAttachment.imageView = SwapchainManager::GetVkSwapchain().depth_image_view;
    depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
    depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthStencilAttachment.clearValue.depthStencil = { depth,  0 };

    VkRenderingInfoKHR renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    renderingInfo.renderArea = { 0, 0, SwapchainManager::GetVkSwapchain().swapchain_extent.width, SwapchainManager::GetVkSwapchain().swapchain_extent.height };
    renderingInfo.layerCount = 1;
    renderingInfo.viewMask = 0;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = &depthStencilAttachment;
    renderingInfo.pStencilAttachment = &depthStencilAttachment;

    // Begin dynamic rendering
    vkCmdBeginRenderingKHR(current_frame.main_command_buffer, &renderingInfo);
}


void VkContext::UpdateBuffer(const std::string& pipeline_name, int set_index,int binding, void* data, size_t data_size) {

    auto& program = *utils::FindUnorderedMap(pipeline_name, shader_program);
    auto& alloc_buffers = *utils::FindUnorderedMap(set_index, program.alloc_buffers);
    //and copy it to the buffer
	void* p_data;
	vmaMapMemory(DeviceManager::GetVkDevice().allocator, alloc_buffers[binding].allocation, &p_data);

	memcpy(p_data, data, data_size);

	vmaUnmapMemory(DeviceManager::GetVkDevice().allocator, alloc_buffers[binding].allocation);
}

void VkContext::BindPipeline(const std::string& pipeline_name) {

    current_pipeline = pipeline_name;
    auto& current_frame = CommandbufferManager::GetCurrentFrame(frame_number);

    auto& program = utils::FindUnorderedMap(current_pipeline, shader_program)->program;

    program.viewport.width = SwapchainManager::GetVkSwapchain().swapchain_extent.width;
    program.viewport.height = SwapchainManager::GetVkSwapchain().swapchain_extent.height;
    program.viewport.x = 0.0f;
    program.viewport.y = 0.0f;
    program.viewport.minDepth = 0.0f;
    program.viewport.maxDepth = 1.0f;
    
    program.scissor.extent = SwapchainManager::GetVkSwapchain().swapchain_extent;
    program.scissor.offset = {0, 0};

    vkCmdSetViewport(current_frame.main_command_buffer, 0, 1, &program.viewport);
    vkCmdSetScissor(current_frame.main_command_buffer, 0, 1, &program.scissor);

    vkCmdBindPipeline(current_frame.main_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, program.pipeline);

}

void VkContext::BindPushConstants(const void *p_values) {
    auto& current_frame = CommandbufferManager::GetCurrentFrame(frame_number);
    auto& program = utils::FindUnorderedMap(current_pipeline, shader_program)->program;
    if(program.push_constants.size() > 0)
    {
        for(auto& current_pconstant: program.push_constants)
        {
            vkCmdPushConstants(current_frame.main_command_buffer, program.layout, current_pconstant.stageFlags, 0, current_pconstant.size, p_values);
        }
    }
}

void VkContext::BindDescriptorSets() {
    auto& current_frame = CommandbufferManager::GetCurrentFrame(frame_number);
    auto& program = utils::FindUnorderedMap(current_pipeline, shader_program)->program;
    for(int i = 0; i < program.descriptor_sets.size(); i++){
        if(program.descriptor_sets[i].set == VK_NULL_HANDLE){
            program.descriptor_builder.Build(program.descriptor_sets[i].set);
        }
        else {
            vkCmdBindDescriptorSets(current_frame.main_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, program.layout, i, 1, &program.descriptor_sets[i].set, 0, 0);
        }
    }
}

void VkContext::Draw() {
    auto& current_frame = CommandbufferManager::GetCurrentFrame(frame_number);
    vkCmdDraw(current_frame.main_command_buffer, 3, 1, 0, 0);
}


void VkContext::EndRenderLayer() {
    auto& current_frame = CommandbufferManager::GetCurrentFrame(frame_number);
    auto& device = DeviceManager::GetVkDevice().device;

    // End dynamic rendering
    vkCmdEndRenderingKHR(current_frame.main_command_buffer);
    // Transition color image for presentation
    vktools::InsertImageMemoryBarrier(
        current_frame.main_command_buffer,
        SwapchainManager::GetVkSwapchain().swapchain_images[image_index],
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
}

void VkContext::SubmitFrame() {

    auto& current_frame = CommandbufferManager::GetCurrentFrame(frame_number);
    auto& device = DeviceManager::GetVkDevice().device;

    //finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK_RESULT(vkEndCommandBuffer(current_frame.main_command_buffer));

	//prepare the submission to the queue. 
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished

	VkSubmitInfo submit = vkdefaults::SubmitInfo(&current_frame.main_command_buffer);
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;

	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &current_frame.present_semaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &current_frame.render_semaphore;

	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CHECK_RESULT(vkQueueSubmit(DeviceManager::GetVkDevice().graphics_queue, 1, &submit, current_frame.render_fence));

	//prepare present
	// this will put the image we just rendered to into the visible window.
	// we want to wait on the _renderSemaphore for that, 
	// as its necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = vkdefaults::PresentInfo();

	presentInfo.pSwapchains = &SwapchainManager::GetVkSwapchain().swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &current_frame.render_semaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &image_index;

	VkResult draw_result = vkQueuePresentKHR(DeviceManager::GetVkDevice().graphics_queue, &presentInfo);

	if (draw_result == VK_ERROR_OUT_OF_DATE_KHR || draw_result == VK_SUBOPTIMAL_KHR || Window::IsFrameBufferResized()) {
		Window::SetFrameBufferResizeState(false);
		RecreateSwapchain();
		
	} else if (draw_result != VK_SUCCESS) {
		ENGINE_CORE_ERROR("failed to present swap chain image!");
		abort();
	}
    
    //increase the number of frames drawn
	frame_number ++;
}

void VkContext::RecreateSwapchain() {
    vkDeviceWaitIdle(DeviceManager::GetVkDevice().device);
    SwapchainManager::DestroySwapchain();
    SwapchainManager::Init();
}

void VkContext::DestroyBuffer(const std::string& pipeline_name, int set_index, int index) {
    auto& program = *utils::FindUnorderedMap(pipeline_name, shader_program);
    auto& buffers = *utils::FindUnorderedMap(set_index, program.alloc_buffers);
    vmaDestroyBuffer(DeviceManager::GetVkDevice().allocator,buffers[index].buffer, buffers[index].allocation);
}

void VkContext::DestroyContext() {
    vkDeviceWaitIdle(DeviceManager::GetVkDevice().device);
    auto& program = *utils::FindUnorderedMap("triangle", shader_program);
    for(int i = 0; i < program.program.descriptor_sets.size(); i++) {
        if(utils::FindUnorderedMap(i, program.alloc_buffers) != nullptr){
            auto& buffers = *utils::FindUnorderedMap(i, program.alloc_buffers);
            for(int i_buffers = 0; i < buffers.size(); i_buffers++) {
                DestroyBuffer("triangle", i,i_buffers);
            }
        }
    }
    program.program.DestroyProgram();
    PipelineBuilder::CleanUp();
    CommandbufferManager::Destroy();
    SwapchainManager::DestroySwapchain();
    DeviceManager::Destroy();
    ENGINE_CORE_INFO("vulkan context destroyed");
}
