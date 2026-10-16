cmake_minimum_required(VERSION 3.28)

project(basalt-kiln
    VERSION 26.3.0
    DESCRIPTION "Kiln: compiles the Lua of a Basalt plugin to bytecode"
    LANGUAGES C CXX
)

option(BASALT_BUILD_TESTS "Build the test executables" ON)
option(BASALT_WERROR "Treat warnings as errors" OFF)
set(BASALT_SANITIZE "" CACHE STRING "Comma-separated sanitizers: address, undefined, thread")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/wire/CMakeLists.txt)
    message(FATAL_ERROR "wire/ is empty. Run: git submodule update --init")
endif()

if(BASALT_BUILD_TESTS)
    enable_testing()
endif()

add_subdirectory(wire)

add_library(kiln-core STATIC src/Compiler.cpp)
target_include_directories(kiln-core PUBLIC src)
target_link_libraries(kiln-core PUBLIC basalt::lua basalt::warnings basalt::sanitizers basalt::parity)

add_executable(kiln src/Main.cpp)
target_link_libraries(kiln PRIVATE kiln-core)
