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

void vktools::UploadData(const VmaAllocation& allocation, void* data, uint32_t data_size, size_t len, size_t byteOffset /*= 0*/)
{
    // Note: pData is not initialized because vmaMapMemory(...) allocates memory for pData. Similarly,
    // vmaUnmapMemory deallocates the memory.
    char* pData;
    vmaMapMemory(DeviceManager::GetVkDevice().allocator, allocation, (void**)&pData);
    // Forward pointer
    pData += byteOffset;
    memcpy(pData, data, len * data_size);
    vmaUnmapMemory(DeviceManager::GetVkDevice().allocator, allocation);
}