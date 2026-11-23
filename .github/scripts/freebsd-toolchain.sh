#!/bin/sh
set -eu

root="$(pwd)/build/freebsd-toolchain"
prefix="$root/install"
llvm=/usr/local/llvm21
version="$("$llvm/bin/llvm-config" --version)"

if [ ! -f "$prefix/built-$version" ]; then
    mkdir -p "$root"
    rm -rf "$root/source" "$root/build" "$prefix"
    git clone -q --depth 1 --filter=blob:none --sparse --branch "llvmorg-$version" \
        https://github.com/llvm/llvm-project "$root/source"
    git -C "$root/source" sparse-checkout set runtimes libcxx libcxxabi libunwind cmake llvm/cmake llvm/utils third-party libc
    cmake -S "$root/source/runtimes" -B "$root/build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER="$llvm/bin/clang" -DCMAKE_CXX_COMPILER="$llvm/bin/clang++" \
        -DCMAKE_INSTALL_PREFIX="$prefix" -DLLVM_ENABLE_RUNTIMES='libcxx;libcxxabi' \
        -DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=OFF \
        -DLLVM_INCLUDE_TESTS=OFF -DLIBCXX_INCLUDE_TESTS=OFF -DLIBCXX_INCLUDE_BENCHMARKS=OFF \
        -DLIBCXXABI_INCLUDE_TESTS=OFF -DLIBCXXABI_USE_LLVM_UNWINDER=OFF -DLIBCXX_CXX_ABI=libcxxabi \
        -DLIBCXX_ENABLE_SHARED=OFF -DLIBCXXABI_ENABLE_SHARED=OFF
    cmake --build "$root/build" --target install-cxx install-cxxabi -j 3
    touch "$prefix/built-$version"
fi

cxx="$(find "$prefix" -name libc++.a | head -1)"
abi="$(find "$prefix" -name libc++abi.a | head -1)"
[ -n "$cxx" ] && [ -n "$abi" ]
cat >"$root/toolchain.cmake" <<TOOLCHAIN
set(CMAKE_C_COMPILER "$llvm/bin/clang" CACHE FILEPATH "")
set(CMAKE_CXX_COMPILER "$llvm/bin/clang++" CACHE FILEPATH "")
set(CMAKE_CXX_FLAGS_INIT "-nostdinc++ -isystem $prefix/include/c++/v1")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-nostdlib++")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-nostdlib++")
set(CMAKE_CXX_STANDARD_LIBRARIES "$cxx $abi -lgcc_s -lpthread -lm" CACHE STRING "")
TOOLCHAIN

cat >"$root/probe.cpp" <<'PROBE'
#include <charconv>
#include <string_view>
int main() {
    const std::string_view text = "1.25";
    double value = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    return parsed.ec != std::errc{} || value != 1.25;
}
PROBE
"$llvm/bin/clang++" -std=c++23 -nostdinc++ -isystem "$prefix/include/c++/v1" -nostdlib++ \
    "$root/probe.cpp" "$cxx" "$abi" -lgcc_s -lpthread -lm -o "$root/probe"
"$root/probe"
ldd "$root/probe"
