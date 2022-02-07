#pragma once 
#define VK_NO_PROTOTYPES
#include "volk.h"
#include "vk_mem_alloc.h"

class Device {
public: 
    VkDevice device;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device;
    VkPhysicalDeviceProperties gpu_properties;

    VkQueue graphics_queue;
    uint32_t graphics_queue_family;

    VmaAllocator allocator;
};

class DeviceManager {
public:
    static void Init();
    static Device& GetVkDevice() {return vk_device;}
    static void Destroy();
private:
    static inline VkInstance instance;
    static inline VkDebugUtilsMessengerEXT debugMessenger;
    static inline Device vk_device;

    static void CreateInstance();
};