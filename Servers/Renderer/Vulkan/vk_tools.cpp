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

VkBool32 vktools::GetSupportedDepthFormat(VkPhysicalDevice& physical_device, VkFormat* depth_format) {
    // Since all depth formats may be optional, we need to find a suitable depth format to use
    // Start with the highest precision packed format
    std::vector<VkFormat> depthFormats = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM
    };

    for (auto& format : depthFormats)
    {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &formatProps);
        // Format must support depth stencil attachment for optimal tiling
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            *depth_format = format;
            return true;
        }
    }

    return false;
}