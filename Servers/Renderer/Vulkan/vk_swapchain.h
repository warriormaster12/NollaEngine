#pragma once

#include "vk_tools.h"
#include <vector>

class Swapchain {
public: 
    VkFormat swapchain_image_format;
    VkFormat swapchain_depth_format;
    VkExtent2D swapchain_extent;
    std::vector<VkImageView> swapchain_image_views;
    VkImageView depth_image_view; 
    std::vector<VkImage> swapchain_images;

    VkSwapchainKHR swapchain;

    vktools::AllocatedImage depth_image;
};

class SwapchainManager {
public: 
    static void Init(VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR);
    static Swapchain& GetVkSwapchain() {return vk_swapchain;}
    static void DestroySwapchain();
private: 
    static inline Swapchain vk_swapchain;
};