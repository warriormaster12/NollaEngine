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
    Renderer::CreateShaderProgram("triangle",{"Shaders/triangle_vert.spv", "Shaders/triangle_frag.spv"});
    Renderer::CreateBuffer("triangle",0,sizeof(TestBufferData), UNIFORM);
    Renderer::CreateBuffer("triangle",1,sizeof(TestBufferData), UNIFORM);
    Renderer::BuildDescriptors("triangle");
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

            TestBufferData test_data2;
            test_data2.color = glm::vec4(0.1f, 0.1f ,0.1f,0.0f);
            Renderer::UpdateBuffer("triangle", 0,0, &test_data, sizeof(TestBufferData));
            Renderer::UpdateBuffer("triangle", 1,0, &test_data2, sizeof(TestBufferData));
            Renderer::BindShaderProgram("triangle");
            Renderer::BindPushConstants(&data);
            Renderer::BindDescriptorSets();
            Renderer::Draw();
            Renderer::EndRenderLayer();
        });    
    }
    Renderer::DestroyBuffer("triangle", 0,0);
    Renderer::DestroyBuffer("triangle", 1,0);
    Renderer::DestroyShaderProgram("triangle");
    Renderer::Destroy();
    Window::DestroyWindow();
    return 0;
}