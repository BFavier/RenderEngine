cmake_minimum_required(VERSION 3.28.1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(RenderEngine)

find_package(glfw3 CONFIG REQUIRED)
find_package(VulkanLoader CONFIG REQUIRED)
find_package(vulkan-validationlayers CONFIG REQUIRED)

# add_library(${PROJECT_NAME} SHARED
add_executable(${PROJECT_NAME}
    src/RenderEngine/graphics/GPU.cpp
    src/RenderEngine/graphics/Image.cpp
    src/RenderEngine/graphics/ImageView.cpp
    src/RenderEngine/graphics/Pipeline.cpp
    src/RenderEngine/graphics/Shader.cpp
    src/RenderEngine/graphics/SwapChain.cpp
    src/RenderEngine/user_interface/Handles.cpp
    src/RenderEngine/user_interface/Keyboard.cpp
    src/RenderEngine/user_interface/Mouse.cpp
    src/RenderEngine/user_interface/Timer.cpp
    src/RenderEngine/user_interface/Window.cpp
    src/RenderEngine/utilities/Functions.cpp
    src/RenderEngine/Engine.cpp
    src/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE include/)
target_link_libraries(${PROJECT_NAME} glfw Vulkan::Loader vulkan-validationlayers::vulkan-validationlayers)

# add_executable(test src/main.cpp)
# target_link_libraries(test ${PROJECT_NAME})
# target_include_directories(test PRIVATE include/)
# target_include_directories(${PROJECT_NAME} PRIVATE GLFW3)
# target_include_directories(${PROJECT_NAME} PRIVATE Vulkan)