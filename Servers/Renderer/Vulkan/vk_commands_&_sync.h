#pragma once 

#include "vk_tools.h"
#include "logger.h"
#include <vector>


struct PerFrameData {
    VkCommandBuffer main_command_buffer;
    VkFence render_fence;
    VkSemaphore present_semaphore, render_semaphore;
    VkCommandPool main_command_pool;

    void CleanUp();
};


constexpr unsigned int FRAME_OVERLAP = 2;


class CommandbufferManager {
public: 
    static void Init();
    static void Destroy();

    static PerFrameData& GetCurrentFrame(int frame_number) { return per_frame_data[frame_number % FRAME_OVERLAP]; }
    static int GetFrameOverlap() {return FRAME_OVERLAP;}
private:
    static void BuildCommands();
    static void BuildSyncStructures();

    static inline PerFrameData per_frame_data[FRAME_OVERLAP];
    
};