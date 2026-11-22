#include <basalt/proxy.h>
#pragma once
#include <basalt/proxy/commands.h>
#include <basalt/proxy/core.hpp>

namespace basalt::proxy::commands {
using namespace basalt::proxy::core;

inline int command(const char* spec, const char* permission, Handler handler) {
    return bs_command(spec, permission, handler);
}

inline int command_reply(Ctx ctx, const char* text, const Args* args = nullptr) {
    return bs_command_reply(ctx, text, core::detail::raw(args));
}

inline Text<256> command_arg_str(Ctx ctx, const char* name) {
    return core::detail::fill<256>([&](char* out, int cap) { return bs_command_arg_str(ctx, name, out, cap); });
}

inline int command_arg_i64(Ctx ctx, const char* name, long long* out) { return bs_command_arg_i64(ctx, name, out); }

inline Text<256> command_source_name(Ctx ctx) {
    return core::detail::fill<256>([&](char* out, int cap) { return bs_command_source_name(ctx, out, cap); });
}

inline bool command_is_console(Ctx ctx) { return bs_command_is_console(ctx) != 0; }
} // namespace basalt::proxy::commands
