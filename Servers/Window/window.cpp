#include "window.h"

#define GLFW_INCLUDE_VULKAN
#define VK_NO_PROTOTYPES
#include <GLFW/glfw3.h> 

#include "logger.h"


void FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(static_cast<GLFWwindow*>(window)));
    app->SetFrameBufferResizeState(true);
}


void Window::CreateNewWindow(uint32_t width /*= 1280*/, uint32_t height /*= 720*/){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* glfw_p_window;
    glfw_p_window = glfwCreateWindow(width,height, "Nolla Engine", nullptr, nullptr);

    p_window = glfw_p_window;

    glfwSetFramebufferSizeCallback(static_cast<GLFWwindow*>(p_window), FramebufferResizeCallback);

    ENGINE_CORE_INFO("new window created");
}

Window::FrameBufferSize& Window::GetFrameBufferSize() {
    glfwGetFramebufferSize(static_cast<GLFWwindow*>(p_window), &framebuffer_size.width, &framebuffer_size.height);

    return framebuffer_size;   
}

bool Window::GetWindowStatus() {
    if (!glfwWindowShouldClose(static_cast<GLFWwindow*>(p_window))){
        glfwPollEvents();
        window_status = WindowStatus::Opened;
        return true;
    }
    else {
        window_status = WindowStatus::Closed;
        return false;
    }
}

void Window::SetWindowStatus(WindowStatus status){
    window_status = status;
    glfwSetWindowShouldClose(static_cast<GLFWwindow*>(p_window), window_status);
}


void Window::DestroyWindow(){
    glfwDestroyWindow(static_cast<GLFWwindow*>(p_window));
    glfwTerminate();

    ENGINE_CORE_INFO("new window destroyed");
}


