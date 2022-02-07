#include "vk_swapchain.h"
#include "window.h"

#define VK_NO_PROTOTYPES
#include "VkBootstrap.h"

#include "logger.h"

#include "vk_device.h"

#include "vk_defaults.h"

#include "vk_tools.h"

void SwapchainManager::Init(VkPresentModeKHR present_mode /*= VK_PRESENT_MODE_IMMEDIATE_KHR*/) {
    auto& framebuffer_size = Window::GetFrameBufferSize();
    vkb::SwapchainBuilder swapchainBuilder{DeviceManager::GetVkDevice().physical_device,DeviceManager::GetVkDevice().device,DeviceManager::GetVkDevice().surface };
	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.use_default_format_selection()
		//use vsync present mode
		.set_desired_present_mode(present_mode)
		.set_desired_extent(static_cast<uint32_t>(framebuffer_size.width), static_cast<uint32_t>(framebuffer_size.height))
		.build()
		.value();

	//store swapchain and its related images
	vk_swapchain.swapchain = vkbSwapchain.swapchain;
	vk_swapchain.swapchain_images = vkbSwapchain.get_images().value();
	vk_swapchain.swapchain_image_views = vkbSwapchain.get_image_views().value();
	vk_swapchain.swapchain_image_format = vkbSwapchain.image_format;

	//we get actual resolution of the displayed content
	vk_swapchain.swapchain_extent = vkbSwapchain.extent;
	//depth image size will match the window
	VkExtent3D depth_image_extent = {
		vk_swapchain.swapchain_extent.width,
		vk_swapchain.swapchain_extent.height,
		1
	};

	//hardcoding the depth format to 32 bit float
	vk_swapchain.swapchain_depth_format = VK_FORMAT_D32_SFLOAT;

	//the depth image will be a image with the format we selected and Depth Attachment usage flag
	VkImageCreateInfo dimg_info = vkdefaults::ImageCreateInfo(vk_swapchain.swapchain_depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depth_image_extent);

	//for the depth image, we want to allocate it from gpu local memory
	VmaAllocationCreateInfo dimg_allocinfo = {};
	dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//allocate and create the image
	vmaCreateImage(DeviceManager::GetVkDevice().allocator, &dimg_info, &dimg_allocinfo, &vk_swapchain.depth_image.image, &vk_swapchain.depth_image.allocation, nullptr);

	//build a image-view for the depth image to use for rendering
	VkImageViewCreateInfo dview_info = vkdefaults::ImageViewCreateInfo(vk_swapchain.swapchain_depth_format, vk_swapchain.depth_image.image, VK_IMAGE_ASPECT_DEPTH_BIT);;

	VK_CHECK_RESULT(vkCreateImageView(DeviceManager::GetVkDevice().device, &dview_info, nullptr, &vk_swapchain.depth_image_view));

    ENGINE_CORE_INFO("vulkan swapchain created");
}

void SwapchainManager::DestroySwapchain() {
	vkDestroyImageView(DeviceManager::GetVkDevice().device, vk_swapchain.depth_image_view, nullptr);
    vmaDestroyImage(DeviceManager::GetVkDevice().allocator, vk_swapchain.depth_image.image, vk_swapchain.depth_image.allocation);
    for (int i = 0; i < vk_swapchain.swapchain_image_views.size(); i++)
    {
        vkDestroyImageView(DeviceManager::GetVkDevice().device,  vk_swapchain.swapchain_image_views[i], nullptr);
    }

    vkDestroySwapchainKHR(DeviceManager::GetVkDevice().device,vk_swapchain.swapchain, nullptr);

    ENGINE_CORE_INFO("vulkan swapchain destroyed");
}