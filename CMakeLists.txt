cmake_minimum_required(VERSION 3.28)

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/wire/CMakeLists.txt)
    message(FATAL_ERROR "wire/ is empty. Run: git submodule update --init")
endif()
include(${CMAKE_CURRENT_SOURCE_DIR}/wire/cmake/BasaltVcpkg.cmake)

project(basalt-kiln
    VERSION 26.3.0
    DESCRIPTION "Kiln: builds, checks and packs WebAssembly plugins for Basalt"
    LANGUAGES C CXX
)

set(BASALT_VERSION "26.3.0-dev" CACHE STRING "Version stamped into the binary")
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

if(BASALT_BUILD_TESTS)
    enable_testing()
endif()

add_subdirectory(wire)

include(cmake/KilnAbi.cmake)
kiln_generate_abi(${CMAKE_CURRENT_SOURCE_DIR}/wire/wire-wasm/abi ${CMAKE_CURRENT_BINARY_DIR}/AbiTable.cpp)

include(cmake/KilnEmbed.cmake)
kiln_embed_tree(${CMAKE_CURRENT_BINARY_DIR}/EmbeddedSdk.cpp kSdkFiles ${CMAKE_CURRENT_SOURCE_DIR}/sdk)
kiln_embed_tree(${CMAKE_CURRENT_BINARY_DIR}/EmbeddedTemplates.cpp kTemplateFiles ${CMAKE_CURRENT_SOURCE_DIR}/templates)
kiln_embed_tree(${CMAKE_CURRENT_BINARY_DIR}/EmbeddedAbi.cpp kAbiFiles ${CMAKE_CURRENT_SOURCE_DIR}/wire/wire-wasm/abi)

add_library(kiln-core STATIC
    src/Archive.cpp src/Build.cpp src/Commands.cpp src/FileLock.cpp src/Http.cpp src/Languages.cpp src/Pack.cpp
    src/Plugin.cpp src/Process.cpp src/Project.cpp src/Scaffold.cpp src/Sha256.cpp src/Toolchain.cpp
    src/ToolchainPins.cpp src/Tools.cpp src/Update.cpp src/UpdateInstall.cpp src/UpdateRelease.cpp
    src/Wasm.cpp src/WasmCheck.cpp src/Xz.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AbiTable.cpp ${CMAKE_CURRENT_BINARY_DIR}/EmbeddedSdk.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/EmbeddedTemplates.cpp ${CMAKE_CURRENT_BINARY_DIR}/EmbeddedAbi.cpp)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/packaging/update-public.pem" KILN_UPDATE_PUBLIC_KEY)
configure_file(packaging/UpdateKey.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/UpdateKey.hpp @ONLY)
target_include_directories(kiln-core PUBLIC src PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(kiln-core PUBLIC basalt::wire-plugins basalt::mbedtls basalt::zlib basalt::platform basalt::warnings
    basalt::sanitizers basalt::parity)
if(WIN32)
    target_link_libraries(kiln-core PUBLIC crypt32 ws2_32)
endif()

add_executable(kiln src/Main.cpp src/BuildInfo.cpp)
set_source_files_properties(src/BuildInfo.cpp PROPERTIES COMPILE_DEFINITIONS BASALT_VERSION="${BASALT_VERSION}")
target_link_libraries(kiln PRIVATE kiln-core)

basalt_test(kiln-wasm-test SOURCES test/WasmTest.cpp DEPENDS kiln-core)
basalt_test(kiln-update-test SOURCES test/UpdateTest.cpp DEPENDS kiln-core)

find_package(Python3 COMPONENTS Interpreter QUIET)
if(BASALT_BUILD_TESTS AND Python3_FOUND)
    add_test(NAME kiln-sdk-generated COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen-sdk.py --check)
    add_test(NAME kiln-worlds-generated COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen-worlds.py --check)
endif()

include(cmake/KilnPackaging.cmake)
