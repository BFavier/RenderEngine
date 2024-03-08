cmake_minimum_required(VERSION 3.28.1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(RenderEngine)

# find_package(glfw3 REQUIRED)
# find_package(VulkanLoader REQUIRED)

# add_library(${PROJECT_NAME} SHARED
add_executable(${PROJECT_NAME}
    src/RenderEngine/graphics/Buffer.cpp
    src/RenderEngine/graphics/Canvas.cpp
    src/RenderEngine/graphics/GPU.cpp
    src/RenderEngine/graphics/Image.cpp
    src/RenderEngine/graphics/Buffer.cpp
    src/RenderEngine/graphics/SwapChain.cpp
    src/RenderEngine/graphics/shaders/Shader.cpp
    src/RenderEngine/graphics/shaders/DemoShader.cpp
    src/RenderEngine/graphics/shaders/Shader3D.cpp
    src/RenderEngine/graphics/model/Mesh.cpp
    src/RenderEngine/user_interface/Window.cpp
    src/RenderEngine/user_interface/Keyboard.cpp
    src/RenderEngine/user_interface/Mouse.cpp
    src/RenderEngine/user_interface/Timer.cpp
    src/RenderEngine/user_interface/Window.cpp
    src/RenderEngine/utilities/Functions.cpp
    src/RenderEngine/Internal.cpp
    src/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE include/)
target_link_libraries(${PROJECT_NAME}
    "C:/glfw-3.3.9.bin.WIN64/lib-vc2022/glfw3.lib"
    "C:/VulkanSDK/1.3.275.0/Lib/vulkan-1.lib")

# add_executable(test src/main.cpp)
# target_link_libraries(test ${PROJECT_NAME})
# target_include_directories(test PRIVATE include/)
# target_include_directories(${PROJECT_NAME} PRIVATE GLFW3)
# target_include_directories(${PROJECT_NAME} PRIVATE Vulkan)