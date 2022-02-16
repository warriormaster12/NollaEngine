#pragma once 

#include <cstdint>
#include <iostream>
#include <array>
#include <vector>
#include <string>


class VkContext {
public: 
    static void InitContext();
    static void CreatePipeline(const std::string& pipeline_name, std::vector<std::string> filepaths, uint32_t stride, std::vector<uint32_t> offsets);
    static void CreateDescriptorBuffer(const std::string& pipeline_name,int set_index,size_t alloc_size, int usage);
    static void CreateBuffer(const std::string& buffer_name, void* data, uint32_t stride, size_t alloc_size,int usage);
    static void BuildDescriptors(const std::string& pipeline_name);
    static void PrepareFrame();
    static void BeginNewRenderLayer(std::array<float, 4> color, float depth);
    static void UpdateBuffer(const std::string& pipeline_name, int set_index,int binding, void* data, size_t data_size);
    static void BindPipeline(const std::string& pipeline_name);
    static void BindPushConstants(const void* p_values);
    static void BindDescriptorSets();
    static void BindVertexBuffers();
    static void BindIndexBuffers();
    static void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex = 0, uint32_t first_instance = 0);
    static void DrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index = 0, uint32_t vertex_offset = 0,uint32_t first_instance = 0);
    static void EndRenderLayer();
    static void SubmitFrame();
    static void DestroyPipeline(const std::string& pipeline_name);
    static void DestroyDescriptorBuffer(const std::string& pipeline_name, int set_index, int index);
    static void DestroyBuffer(const std::string& buffer_name);
    static void DestroyContext();
private:
    static inline int frame_number = 0;
    static inline uint32_t image_index = 0;

    static inline uint32_t result;

    static inline std::string current_pipeline;

    static void RecreateSwapchain();
};
