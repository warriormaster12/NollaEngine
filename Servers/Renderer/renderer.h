#pragma once 

#include <iostream>
#include <array>
#include <vector>
#include <functional>

class Renderer {
public: 
    static void Init();
    static void CreateShaderProgram(std::vector<std::string> filepaths);

    static void InsertDrawCalls(std::function<void()>&& drawCalls);
    static void BeginNewRenderLayer(std::array<float, 4> color, float depth);
    static void BindShaderProgram();
    static void Draw();

    static void EndRenderLayer();
    static void Destroy();
};