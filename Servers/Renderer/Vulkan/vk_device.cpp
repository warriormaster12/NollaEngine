#define VOLK_IMPLEMENTATION
#define VK_NO_PROTOTYPES
#include "volk.h"
#include "VkBootstrap.h"

#include "vk_device.h"

#include "logger.h"

#include "vk_tools.h"

#include <GLFW/glfw3.h>
#include "window.h"



void DeviceManager::Init(){
    volkInitialize();


    vkb::InstanceBuilder builder;

    auto inst_ret = builder.set_app_name("Nolla Engine")
    .request_validation_layers(true)
    .use_default_debug_messenger()
    .build();

    vkb::Instance vkb_inst = inst_ret.value();

    instance = vkb_inst.instance;
    volkLoadInstance(instance);
    ENGINE_CORE_INFO("vulkan instance created");

    debugMessenger = vkb_inst.debug_messenger;

    VkPhysicalDeviceFeatures feats{};
    
    VkPhysicalDeviceDynamicRenderingFeatures dynamic_features = {};
    dynamic_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    dynamic_features.dynamicRendering = VK_TRUE;

    vkb::PhysicalDeviceSelector selector{ vkb_inst };

    selector.set_required_features(feats);
    selector.add_desired_extension(VK_KHR_MAINTENANCE2_EXTENSION_NAME)
    .add_desired_extension(VK_KHR_MULTIVIEW_EXTENSION_NAME)
    .add_desired_extension(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME)
    .add_desired_extension(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME)
    .add_desired_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    
    glfwCreateWindowSurface(instance, static_cast<GLFWwindow*>(Window::GetWindowPointer()), nullptr, &vk_device.surface);
    
    auto physical_device = selector
		.set_minimum_version(1, 3)
        .set_surface(vk_device.surface)
		.select();
    
    if (!physical_device) {
        ENGINE_CORE_FATAL("failed to select vulkan compatible gpu");
        abort();
    }
    vkb::DeviceBuilder deviceBuilder{ physical_device.value()};
	vkb::Device vkb_device = deviceBuilder.add_pNext(&dynamic_features).build().value();
	// Get the VkDevice handle used in the rest of a vulkan application
	vk_device.device = vkb_device.device;
	volkLoadDevice(vk_device.device);
    vk_device.physical_device = physical_device.value().physical_device;

    vk_device.graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();

	vk_device.graphics_queue_family = vkb_device.get_queue_index(vkb::QueueType::graphics).value();
    
    
    //we need to provide instance and device address so that vma library could work properly with volk
    VmaVulkanFunctions volk_functions = {};
    volk_functions.vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)vkGetDeviceProcAddr;
    volk_functions.vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)vkGetInstanceProcAddr;
    volk_functions.vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)vkGetPhysicalDeviceProperties;
    volk_functions.vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)vkGetPhysicalDeviceMemoryProperties;
    volk_functions.vkAllocateMemory = (PFN_vkAllocateMemory)vkAllocateMemory;
    volk_functions.vkFreeMemory = (PFN_vkFreeMemory)vkFreeMemory;
    volk_functions.vkMapMemory = (PFN_vkMapMemory)vkMapMemory;
    volk_functions.vkUnmapMemory = (PFN_vkUnmapMemory)vkUnmapMemory;
    volk_functions.vkGetDeviceBufferMemoryRequirements = (PFN_vkGetDeviceBufferMemoryRequirements)vkGetDeviceBufferMemoryRequirements;
    volk_functions.vkGetDeviceImageMemoryRequirements = (PFN_vkGetDeviceImageMemoryRequirements)vkGetDeviceImageMemoryRequirements;
    volk_functions.vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)vkFlushMappedMemoryRanges;
    volk_functions.vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges)vkInvalidateMappedMemoryRanges;
    volk_functions.vkBindBufferMemory = (PFN_vkBindBufferMemory)vkBindBufferMemory;
    volk_functions.vkBindImageMemory = (PFN_vkBindImageMemory)vkBindImageMemory;
    volk_functions.vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)vkGetBufferMemoryRequirements;
    volk_functions.vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)vkGetImageMemoryRequirements;
    volk_functions.vkCreateBuffer = (PFN_vkCreateBuffer)vkCreateBuffer;
    volk_functions.vkCreateImage = (PFN_vkCreateImage)vkCreateImage;
    volk_functions.vkDestroyBuffer = (PFN_vkDestroyBuffer)vkDestroyBuffer;
    volk_functions.vkDestroyImage = (PFN_vkDestroyImage)vkDestroyImage;
    volk_functions.vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer)vkCmdCopyBuffer;
    volk_functions.vkGetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2KHR)vkGetBufferMemoryRequirements2KHR;
    volk_functions.vkGetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2KHR)vkGetImageMemoryRequirements2KHR;
    volk_functions.vkBindBufferMemory2KHR = (PFN_vkBindBufferMemory2KHR)vkBindBufferMemory2KHR;
    volk_functions.vkBindImageMemory2KHR = (PFN_vkBindImageMemory2KHR)vkBindImageMemory2KHR;
    volk_functions.vkGetPhysicalDeviceMemoryProperties2KHR = (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)vkGetPhysicalDeviceMemoryProperties2KHR;
    volk_functions.vkGetDeviceBufferMemoryRequirements = (PFN_vkGetDeviceBufferMemoryRequirements)vkGetDeviceBufferMemoryRequirements;
    volk_functions.vkGetDeviceImageMemoryRequirements = (PFN_vkGetDeviceImageMemoryRequirements)vkGetDeviceImageMemoryRequirements;



    //initialize the memory allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = vk_device.physical_device;
    allocatorInfo.device = vk_device.device;
    allocatorInfo.instance = instance;
    allocatorInfo.pVulkanFunctions = &volk_functions;
    vmaCreateAllocator(&allocatorInfo, &vk_device.allocator);
    
    
    vkGetPhysicalDeviceProperties(vk_device.physical_device, &vk_device.gpu_properties);

	ENGINE_CORE_INFO("vulkan device created");
	
	ENGINE_CORE_INFO(physical_device.value().properties.deviceName);
}

void DeviceManager::Destroy() {
    vkDestroySurfaceKHR(instance,vk_device.surface,nullptr);
    vmaDestroyAllocator(vk_device.allocator);
    vkDestroyDevice(vk_device.device, nullptr);
    vkb::destroy_debug_utils_messenger(instance, debugMessenger);
    vkDestroyInstance(instance, nullptr);
    ENGINE_CORE_INFO("vulkan instance destroyed");
}