#include <iostream>
#include "logger.h"
#include <entt.hpp>

#include "window.h"
#include "vk_context.h"

int main(int argc, char* argv[]) 
{
    Logger::Init();
    Window::CreateWindow();
    VkContext::InitContext();
    while (Window::GetWindowStatus()) {
        VkContext::PrepareFrame();

        VkContext::SubmitFrame();
    }
    VkContext::DestroyContext();
    Window::DestroyWindow();
    return 0;
}