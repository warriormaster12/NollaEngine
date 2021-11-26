cmake_minimum_required(VERSION 3.0.0)
project(NollaEngine VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SRC
${PROJECT_SOURCE_DIR}/Main/*
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
    Utils/
    Third-Party/spdlog/include/
    Third-Party/entt/
)



