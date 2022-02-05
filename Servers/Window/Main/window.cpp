#include "window.h"

#define GLFW_INCLUDE_VULKAN
#define VK_NO_PROTOTYPES
#include <GLFW/glfw3.h> 


static GLFWwindow* p_window;

void Window::CreateWindow(uint32_t width /*= 1280*/, uint32_t height /*= 720*/){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    p_window = glfwCreateWindow(1280,720, "Nolla Engine", nullptr, nullptr);
}

bool Window::GetWindowStatus() {
    if (!glfwWindowShouldClose(p_window)){
        window_status = WindowStatus::Opened;
        return true;
    }
    else {
        window_status = WindowStatus::Closed;
        return false;
    }
}

void Window::UpdateWindowEvents(){
    glfwPollEvents();
}

void Window::SetWindowStatus(WindowStatus status){
    window_status = status;
    glfwSetWindowShouldClose(p_window, window_status);
}


void Window::DestroyWindow(){
    glfwDestroyWindow(p_window);
    glfwTerminate();
}