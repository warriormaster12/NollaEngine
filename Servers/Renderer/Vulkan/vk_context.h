#pragma once 

#include <iostream>
#include <array>
#include <vector>


class VkContext {
public: 
    static void InitContext();
    static void PrepareFrame();
    static void BeginNewRenderLayer(std::array<float, 4> color, float depth);
    static void CreatePipeline(std::vector<std::string> filepaths);
    static void EndRenderLayer();
    static void SubmitFrame();
    static void DestroyContext();
private:
    static inline int frame_number = 0;
    static inline uint32_t image_index = 0;

    static inline uint32_t result;

    static void RecreateSwapchain();
};
