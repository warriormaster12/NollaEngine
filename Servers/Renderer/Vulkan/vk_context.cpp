#include "vk_context.h"

#include "logger.h"
#include "vk_device.h"
#include "vk_swapchain.h"
#include "vk_commands_&_sync.h"

#include "vk_tools.h"
#include "vk_defaults.h"

void VkContext::InitContext() {

    DeviceManager::Init();
    SwapchainManager::Init();
    CommandbufferManager::Init();

    ENGINE_CORE_INFO("vulkan context created");
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
		//recreateSwapchain();
        return;
	} else if (draw_result != VK_SUCCESS && draw_result != VK_SUBOPTIMAL_KHR) {
		ENGINE_CORE_ERROR("failed to acquire swap chain image!");
		abort();
	}

    //begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmd_begin_info = vkdefaults::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK_RESULT(vkBeginCommandBuffer(current_frame.main_command_buffer, &cmd_begin_info));


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
    colorAttachment.clearValue.color = { 1.0f,1.0f,0.0f,1.0f };

    // A single depth stencil attachment info can be used, but they can also be specified separately.
    // When both are specified separately, the only requirement is that the image view is identical.			
    VkRenderingAttachmentInfoKHR depthStencilAttachment{};
    depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    depthStencilAttachment.imageView = SwapchainManager::GetVkSwapchain().depth_image_view;
    depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
    depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthStencilAttachment.clearValue.depthStencil = { 1.0f,  0 };

    VkRenderingInfoKHR renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    renderingInfo.renderArea = { 0, 0, SwapchainManager::GetVkSwapchain().swapchain_extent.width, SwapchainManager::GetVkSwapchain().swapchain_extent.height };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = &depthStencilAttachment;
    renderingInfo.pStencilAttachment = &depthStencilAttachment;

    // Begin dynamic rendering
    vkCmdBeginRenderingKHR(current_frame.main_command_buffer, &renderingInfo);


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

	if (draw_result == VK_ERROR_OUT_OF_DATE_KHR || draw_result == VK_SUBOPTIMAL_KHR) {
		// windowHandler.frameBufferResized = false;
		// recreateSwapchain();
		
	} else if (draw_result != VK_SUCCESS) {
		ENGINE_CORE_ERROR("failed to present swap chain image!");
		abort();
	}
    
    //increase the number of frames drawn
	frame_number ++;
}

void VkContext::DestroyContext() {
    vkDeviceWaitIdle(DeviceManager::GetVkDevice().device);
    CommandbufferManager::Destroy();
    SwapchainManager::DestroySwapchain();
    DeviceManager::Destroy();
    ENGINE_CORE_INFO("vulkan context destroyed");
}