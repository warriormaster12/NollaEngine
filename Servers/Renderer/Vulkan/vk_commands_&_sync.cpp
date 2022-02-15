#include "vk_commands_&_sync.h" 
#include "vk_defaults.h"
#include "vk_device.h"
#include "vk_tools.h"


void PerFrameData::CleanUp() {
    auto& device = DeviceManager::GetVkDevice().device;
    vkDestroyCommandPool(device, main_command_pool, nullptr);

    vkDestroyFence(device, render_fence, nullptr);
    vkDestroySemaphore(device, present_semaphore, nullptr);
	vkDestroySemaphore(device, render_semaphore, nullptr);
}

void CommandbufferManager::Init() {
    BuildCommands();
    BuildSyncStructures();
}

void CommandbufferManager::Destroy() {
    for(auto& frame : per_frame_data){
        frame.CleanUp();
    }
	auto& device = DeviceManager::GetVkDevice().device;
	vkDestroyCommandPool(device, upload_context.command_pool, nullptr);
	vkDestroyFence(device, upload_context.upload_fence, nullptr);
}


void CommandbufferManager::BuildCommands() {
    VkCommandPoolCreateInfo command_pool_info = vkdefaults::CommandPoolCreateInfo(DeviceManager::GetVkDevice().graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    for(auto& frame : per_frame_data){
        VK_CHECK_RESULT(vkCreateCommandPool(DeviceManager::GetVkDevice().device, &command_pool_info, nullptr, &frame.main_command_pool));

        VkCommandBufferAllocateInfo cmd_alloc_info = vkdefaults::CommandBufferAllocateInfo(frame.main_command_pool);

        VK_CHECK_RESULT(vkAllocateCommandBuffers(DeviceManager::GetVkDevice().device, &cmd_alloc_info, &frame.main_command_buffer));
    }

	VkCommandPoolCreateInfo upload_command_pool_info = vkdefaults::CommandPoolCreateInfo(DeviceManager::GetVkDevice().graphics_queue_family);
	//create pool for upload context
	VK_CHECK_RESULT(vkCreateCommandPool(DeviceManager::GetVkDevice().device, &upload_command_pool_info, nullptr, &upload_context.command_pool));

    ENGINE_CORE_INFO("vulkan commandbuffers created");
}

void CommandbufferManager::BuildSyncStructures() {
    //create syncronization structures
	//one fence to control when the gpu has finished rendering the frame,
	//and 2 semaphores to syncronize rendering with swapchain
	//we want the fence to start signalled so we can wait on it on the first frame
	VkFenceCreateInfo fenceCreateInfo = vkdefaults::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

	VkSemaphoreCreateInfo semaphoreCreateInfo = vkdefaults::SemaphoreCreateInfo();

	for (auto& frame : per_frame_data) {

		VK_CHECK_RESULT(vkCreateFence(DeviceManager::GetVkDevice().device, &fenceCreateInfo, nullptr, &frame.render_fence));


		VK_CHECK_RESULT(vkCreateSemaphore(DeviceManager::GetVkDevice().device, &semaphoreCreateInfo, nullptr, &frame.present_semaphore));
		VK_CHECK_RESULT(vkCreateSemaphore(DeviceManager::GetVkDevice().device, &semaphoreCreateInfo, nullptr, &frame.render_semaphore));
		
	}

	VkFenceCreateInfo uploadFenceCreateInfo = vkdefaults::FenceCreateInfo();

	VK_CHECK_RESULT(vkCreateFence(DeviceManager::GetVkDevice().device, &uploadFenceCreateInfo, nullptr, &upload_context.upload_fence));
}

void CommandbufferManager::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	//allocate the default command buffer that we will use for the instant commands
	VkCommandBufferAllocateInfo cmdAllocInfo = vkdefaults::CommandBufferAllocateInfo(upload_context.command_pool, 1);

    VkCommandBuffer cmd;
	VK_CHECK_RESULT(vkAllocateCommandBuffers(DeviceManager::GetVkDevice().device, &cmdAllocInfo, &cmd));

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = vkdefaults::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK_RESULT(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    //execute the function
	function(cmd);

	VK_CHECK_RESULT(vkEndCommandBuffer(cmd));

	VkSubmitInfo submit = vkdefaults::SubmitInfo(&cmd);


	//submit command buffer to the queue and execute it.
	// _uploadFence will now block until the graphic commands finish execution
	VK_CHECK_RESULT(vkQueueSubmit(DeviceManager::GetVkDevice().graphics_queue, 1, &submit, upload_context.upload_fence));

	vkWaitForFences(DeviceManager::GetVkDevice().device, 1, &upload_context.upload_fence, true, 9999999999);
	vkResetFences(DeviceManager::GetVkDevice().device, 1, &upload_context.upload_fence);

    //clear the command pool. This will free the command buffer too
	vkResetCommandPool(DeviceManager::GetVkDevice().device, upload_context.command_pool, 0);
}