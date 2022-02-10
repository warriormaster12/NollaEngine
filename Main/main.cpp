#include <iostream>
#include "glm/fwd.hpp"
#include "logger.h"
#include <entt.hpp>
#include <glm/glm.hpp>

#include "window.h"
#include "renderer.h"

struct TestBufferData {
    glm::vec4 color;
};

int main(int argc, char* argv[]) 
{
    Logger::Init();
    Window::CreateNewWindow();
    Renderer::Init();
    Renderer::CreateShaderProgram({"Shaders/triangle_vert.spv", "Shaders/triangle_frag.spv"});
    Renderer::CreateBuffer(sizeof(TestBufferData), UNIFORM);
    while (Window::GetWindowStatus()) {
        Renderer::InsertDrawCalls([](){
            Renderer::BeginNewRenderLayer({0.0f, 1.0f, 0.0f, 1.0f}, 1.0f);
            struct Pushdata
            {
                glm::vec4 color;
            }data;

            data.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
            TestBufferData test_data;
            test_data.color = glm::vec4(1.0f, 0.5f ,0.25f,1.0f);
            Renderer::UpdateBuffer(&test_data, sizeof(TestBufferData));
            Renderer::BindShaderProgram();
            Renderer::BindPushConstants(&data);
            Renderer::BindDescriptorSets();
            Renderer::Draw();
            Renderer::EndRenderLayer();
        });    
    }
    //Renderer::DestroyBuffer();
    Renderer::Destroy();
    Window::DestroyWindow();
    return 0;
}