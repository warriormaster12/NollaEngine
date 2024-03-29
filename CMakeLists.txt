cmake_minimum_required(VERSION 3.16.0)
project(NollaEngine VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SRC
${PROJECT_SOURCE_DIR}/Main/*
${PROJECT_SOURCE_DIR}/Third-Party/vma/*
${PROJECT_SOURCE_DIR}/Third-Party/spirv-reflect/*
${PROJECT_SOURCE_DIR}/Third-Party/vk-bootstrap/*
${PROJECT_SOURCE_DIR}/Servers/*
${PROJECT_SOURCE_DIR}/Scene/*
${PROJECT_SOURCE_DIR}/Utils/*
)

set(BUILD_SHARED_LIBS OFF)
if (WIN32 AND MINGW)
    set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lssp -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")
endif()

add_executable(NollaEngine ${SRC})

if (UNIX AND NOT APPLE)
    set (LINUX TRUE)
    if(DGLFW_USE_WAYLAND=ON)
        set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WAYLAND_KHR)
    else()
        set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
    endif ()
endif ()


target_include_directories(NollaEngine PUBLIC
    Main/
    Third-Party/entt/
    Third-Party/vk-bootstrap/
    Third-Party/volk/
    Third-Party/vma/
    Third-Party/spirv-reflect/
    Third-Party/glm/

    Utils/

    Servers/Window/
    Servers/Renderer/
    Servers/Renderer/Vulkan/
    
    Scene/
)



add_subdirectory(Third-Party/glfw/)

add_subdirectory(Third-Party/spdlog/)

target_link_libraries(NollaEngine PRIVATE glfw)
target_link_libraries(NollaEngine PRIVATE spdlog)

file(COPY ${PROJECT_SOURCE_DIR}/Shaders DESTINATION ${PROJECT_BINARY_DIR})



