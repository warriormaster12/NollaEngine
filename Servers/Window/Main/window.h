#pragma once

#include <iostream>

enum WindowStatus{
    Closed = 0,
    Opened = 1
};


class Window 
{
public: 
    static void CreateWindow(uint32_t width = 1280, uint32_t height = 720);
    static bool GetWindowStatus();
    static void UpdateWindowEvents();
    static void SetWindowStatus(WindowStatus status);
    static void DestroyWindow();

private:
    static inline WindowStatus window_status;


};