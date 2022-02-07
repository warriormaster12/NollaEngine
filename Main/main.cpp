#include <iostream>
#include "logger.h"
#include <entt.hpp>

#include "window.h"
#include "renderer.h"

int main(int argc, char* argv[]) 
{
    Logger::Init();
    Window::CreateNewWindow();
    Renderer::Init();
    Renderer::CreateShaderProgram({"Shaders/triangle_vert.spv", "Shaders/triangle_frag.spv"});
    while (Window::GetWindowStatus()) {
        Renderer::InsertDrawCalls([](){
            Renderer::BeginNewRenderLayer({0.0f, 1.0f, 0.0f, 1.0f}, 1.0f);

            Renderer::EndRenderLayer();
        });    
    }
    Renderer::Destroy();
    Window::DestroyWindow();
    return 0;
}