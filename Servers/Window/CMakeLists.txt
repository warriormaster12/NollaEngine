# Set the project name
project (window)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SRC
${PROJECT_SOURCE_DIR}/Main/*
)

# Add a library with the above sources
add_library(window STATIC ${SRC})


target_include_directories(window PUBLIC
    Main/
)

target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/
)

#glfw
add_subdirectory(${PROJECT_SOURCE_DIR}/Third-Party/glfw EXCLUDE_FROM_ALL)
target_link_libraries(window PRIVATE glfw)
