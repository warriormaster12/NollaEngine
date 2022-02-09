#pragma once 

#define VK_NO_PROTOTYPES
#include "volk.h"

#include "logger.h"
#include "vk_mem_alloc.h"
#include <assert.h>

#include "vk_defaults.h"

#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		ENGINE_CORE_ERROR("Fatal : VkResult is {0} in {1} at line {2}", res, __FILE__, __LINE__); \
		assert(res == VK_SUCCESS);																		\
	}																									\
}

namespace vktools {
	struct AllocatedImage {
		VkImage image = VK_NULL_HANDLE;
		VmaAllocation allocation;
		VkImageView defaultView = VK_NULL_HANDLE;
		int mipLevels;
	};

	struct AllocatedBuffer {
		VkBuffer buffer;
		VmaAllocation allocation;
	};

	void InsertImageMemoryBarrier(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkAccessFlags srcAccessMask,
			VkAccessFlags dstAccessMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask,
			VkImageSubresourceRange subresourceRange
	);

	AllocatedBuffer CreateBuffer(size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage);
	
	
}


