#include <basalt/proxy.h>
#pragma once
#include <basalt/proxy/core.hpp>
#include <basalt/proxy/text.h>

namespace basalt::proxy::text {
using namespace basalt::proxy::core;

inline Text<256> translate(const char* key, const Args* args = nullptr) {
    return core::detail::fill<256>(
        [&](char* out, int cap) { return bs_translate(key, core::detail::raw(args), out, cap); });
}
} // namespace basalt::proxy::text
