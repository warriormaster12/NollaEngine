#pragma once 

#include <iostream>
#include <array>
#include <vector>
#include <string>


class VkContext {
public: 
    static void InitContext();
    static void CreatePipeline(const std::string& pipeline_name,std::vector<std::string> filepaths);
    static void CreateBuffer(const std::string& pipeline_name,size_t alloc_size, int usage);
    static void BuildDescriptors(const std::string& pipeline_name);
    static void PrepareFrame();
    static void BeginNewRenderLayer(std::array<float, 4> color, float depth);
    static void UpdateBuffer(const std::string& pipeline_name, int index, void* data, size_t data_size);
    static void BindPipeline(const std::string& pipeline_name);
    static void BindPushConstants(const void* p_values);
    static void BindDescriptorSets();
    static void Draw();
    static void EndRenderLayer();
    static void SubmitFrame();
    static void DestroyBuffer(const std::string& pipeline_name, int index);
    static void DestroyContext();
private:
    static inline int frame_number = 0;
    static inline uint32_t image_index = 0;

    static inline uint32_t result;

    static inline std::string current_pipeline;

    static void RecreateSwapchain();
};
