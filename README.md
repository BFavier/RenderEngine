# RenderEngine

## Compiling from source

* Install [CMake](https://cmake.org/download/) if it is not installed already
* Download the [LunarG Vulkan SDK](https://vulkan.lunarg.com/) for your target platform
* Download the [glfw precompiled binaries](https://www.glfw.org/download) for your target platform
* Generate the project files and build the project with the following cmake commands, with ${BUILD_TYPE} replaced by Debug or Release
~~~
cd ./build
cmake ..
cmake --build . --config ${BUILD_TYPE}
~~~
