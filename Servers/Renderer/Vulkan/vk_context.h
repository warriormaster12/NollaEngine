#pragma once 

#include <iostream>

class VkContext {
public: 
    static void InitContext();
    static void PrepareFrame();
    static void SubmitFrame();
    static void DestroyContext();
private:
    static inline int frame_number = 0;
    static inline uint32_t image_index = 0;

    static inline uint32_t result;
};
