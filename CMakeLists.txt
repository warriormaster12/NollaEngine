cmake_minimum_required(VERSION 3.0.0)
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
${PROJECT_SOURCE_DIR}/Utils/*
)

set(BUILD_SHARED_LIBS OFF)
if (WIN32 AND MINGW)
    set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lssp -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")
endif()

add_executable(NollaEngine ${SRC})

target_include_directories(NollaEngine PUBLIC
    Main/
    Third-Party/entt/
    Third-Party/vk-bootstrap/
    Third-Party/volk/
    Third-Party/vma/
    Third-Party/spirv-reflect/
    Third-Party/glm/
    Third-Party/spdlog/include/

    Utils/

    Servers/Window/
    Servers/Renderer/
    Servers/Renderer/Vulkan/
)



add_subdirectory(Third-Party/glfw/)

target_link_libraries(NollaEngine PRIVATE glfw)

file(COPY ${PROJECT_SOURCE_DIR}/Shaders DESTINATION ${PROJECT_BINARY_DIR})



