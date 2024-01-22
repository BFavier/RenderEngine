# RenderEngine

## Compiling from source

* Install [CMake](https://cmake.org/download/) if it is not installed already
* Install the [conan](https://conan.io/downloads) C++ package manager
* Setup conan to use the automatically detected compiler
~~~
conan profile detect --force
~~~
* Install dependencies and create build directory with conan
~~~
conan install . --output-folder=build --build=missing
~~~
* build the project with cmake
~~~
cd ./build
cmake .. -G 'Visual Studio 17 2022' -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
cmake --build . --config Release
~~~
