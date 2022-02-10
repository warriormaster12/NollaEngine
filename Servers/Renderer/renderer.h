#pragma once 

#include <iostream>
#include <array>
#include <vector>
#include <functional>

enum BufferTypes {
    UNIFORM = 0x00000010, 
    STORAGE = 0x00000020
};

class Renderer {
public: 
    static void Init();
    static void CreateShaderProgram(std::vector<std::string> filepaths);
    static void CreateBuffer(size_t alloc_size, BufferTypes usage);
    static void InsertDrawCalls(std::function<void()>&& drawCalls);
    static void BeginNewRenderLayer(std::array<float, 4> color, float depth);
    static void UpdateBuffer(void* data, size_t data_size);
    static void BindShaderProgram();
    static void BindDescriptorSets();
    static void BindPushConstants(const void* p_values);
    static void Draw();

    static void EndRenderLayer();
    static void DestroyBuffer();
    static void Destroy();
};