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

    static void CreateWindow(uint32_t width = 1280, uint32_t height = 720);
    static bool GetWindowStatus();
    static void* GetWindowPointer() {return p_window;}
    static void SetWindowStatus(WindowStatus status);
    static void DestroyWindow();

    static FrameBufferSize& GetFrameBufferSize();
    

private:
    static inline WindowStatus window_status;
    static inline void* p_window;
    
    static inline FrameBufferSize framebuffer_size;

};