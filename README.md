# RenderEngine

## Compiling from source

* Install [CMake](https://cmake.org/download/) if it is not installed already
* Install the [conan](https://conan.io/downloads) C++ package manager
* Setup conan to use the automatically detected compiler
~~~
conan profile detect --force
~~~
* Install dependencies and create build directory with conan, with ${BUILD_TYPE} replaced by Debug or Release
~~~
conan install . --output-folder=build --build=missing -s build_type=${BUILD_TYPE}
~~~
* build the project with cmake, with ${BUILD_TYPE} replaced by Debug or Release
~~~
cd ./build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
cmake --build . --config ${BUILD_TYPE}
~~~
