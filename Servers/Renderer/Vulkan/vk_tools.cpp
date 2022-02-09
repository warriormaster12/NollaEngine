#include "vk_tools.h"
#include "vk_device.h"


void vktools::InsertImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange)
{
    VkImageMemoryBarrier imageMemoryBarrier = vkdefaults::ImageMemoryBarrier();
    imageMemoryBarrier.srcAccessMask = srcAccessMask;
    imageMemoryBarrier.dstAccessMask = dstAccessMask;
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;

    vkCmdPipelineBarrier(
        cmdbuffer,
        srcStageMask,
        dstStageMask,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);
}

vktools::AllocatedBuffer vktools::CreateBuffer(size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage) {
    //allocate a buffer
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;

	bufferInfo.size = alloc_size;
	bufferInfo.usage = usage;


	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = memory_usage;

	AllocatedBuffer newBuffer;

	//allocate the buffer
	VK_CHECK_RESULT(vmaCreateBuffer(DeviceManager::GetVkDevice().allocator, &bufferInfo, &vmaallocInfo,
		&newBuffer.buffer,
		&newBuffer.allocation,
		nullptr));

	return newBuffer;
}