#pragma once

#include <iostream>

enum WindowStatus{
    Closed = 0,
    Opened = 1
};


class Window 
{
public: 
    struct FrameBufferSize {
        int width;
        int height;
    };

    static void CreateNewWindow(uint32_t width = 1280, uint32_t height = 720);
    static bool GetWindowStatus();
    static void* GetWindowPointer() {return p_window;}
    static void SetWindowStatus(WindowStatus status);
    static void DestroyWindow();

    static FrameBufferSize& GetFrameBufferSize();

    static bool& IsFrameBufferResized() {return framebuffer_resized;}
    static void SetFrameBufferResizeState(bool input) {framebuffer_resized = input;}
    

private:
    static inline WindowStatus window_status;
    static inline void* p_window;
    
    static inline FrameBufferSize framebuffer_size;
    static inline bool framebuffer_resized = false;

};