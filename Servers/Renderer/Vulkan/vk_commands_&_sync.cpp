#include "vk_commands_&_sync.h" 
#include "vk_defaults.h"
#include "vk_device.h"


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
}


void CommandbufferManager::BuildCommands() {
    VkCommandPoolCreateInfo command_pool_info = vkdefaults::CommandPoolCreateInfo(DeviceManager::GetVkDevice().graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    for(auto& frame : per_frame_data){
        VK_CHECK_RESULT(vkCreateCommandPool(DeviceManager::GetVkDevice().device, &command_pool_info, nullptr, &frame.main_command_pool));

        VkCommandBufferAllocateInfo cmd_alloc_info = vkdefaults::CommandBufferAllocateInfo(frame.main_command_pool);

        VK_CHECK_RESULT(vkAllocateCommandBuffers(DeviceManager::GetVkDevice().device, &cmd_alloc_info, &frame.main_command_buffer));
    }

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
}