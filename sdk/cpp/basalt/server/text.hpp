#include <basalt/server.h>
#pragma once
#include <basalt/server/core.hpp>
#include <basalt/server/text.h>

namespace basalt::server::text {
using namespace basalt::server::core;

inline Text<256> translate(const char* key, const Args* args = nullptr) {
    return core::detail::fill<256>(
        [&](char* out, int cap) { return bs_translate(key, core::detail::raw(args), out, cap); });
}
} // namespace basalt::server::text
