#include <iostream>
#include "logger.h"
#include <entt.hpp>

#include "window.h"

int main(int argc, char* argv[]) 
{
    Logger::Init();
    Window::CreateWindow();
    while (Window::GetWindowStatus()) {
        Window::UpdateWindowEvents();
    }
    ENGINE_CORE_INFO("window closed");
    Window::DestroyWindow();
    return 0;
}