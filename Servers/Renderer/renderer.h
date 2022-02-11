#pragma once 

#include <iostream>
#include <array>
#include <vector>
#include <functional>
#include <string>

enum BufferTypes {
    UNIFORM = 0x00000010, 
    STORAGE = 0x00000020
};

class Renderer {
public: 
    static void Init();
    static void CreateShaderProgram(const std::string& shader_name, std::vector<std::string> filepaths);
    static void CreateBuffer(const std::string& shader_name,int set_index,size_t alloc_size, BufferTypes usage);
    static void BuildDescriptors(const std::string& shader_name);
    static void InsertDrawCalls(std::function<void()>&& drawCalls);
    static void BeginNewRenderLayer(std::array<float, 4> color, float depth);
    static void UpdateBuffer(const std::string& pipeline_name, int set_index,int binding,void* data, size_t data_size);
    static void BindShaderProgram(const std::string& shader_name);
    static void BindDescriptorSets();
    static void BindPushConstants(const void* p_values);
    static void Draw();

    static void EndRenderLayer();
    static void DestroyBuffer(const std::string& pipeline_name,int set_index, int index);
    static void Destroy();
};