#pragma once
#define VK_NO_PROTOTYPES
#include "volk.h"

namespace vkdefaults {
    VkImageCreateInfo ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
    VkImageViewCreateInfo ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

    VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
    VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);

    VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags = 0);
    VkSemaphoreCreateInfo SemaphoreCreateInfo();

    VkSubmitInfo SubmitInfo(VkCommandBuffer* p_cmd);
    VkPresentInfoKHR PresentInfo();

    VkImageMemoryBarrier ImageMemoryBarrier();

    VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shader_module);

    VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo();

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo(VkPrimitiveTopology topology);

    VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo(VkPolygonMode polygon_mode, VkCullModeFlags cull_mode = VK_CULL_MODE_BACK_BIT);

    VkPipelineMultisampleStateCreateInfo MultisamplingStateCreateInfo();

    VkPipelineColorBlendAttachmentState ColorBlendAttachmentState();

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo();

    VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);
}