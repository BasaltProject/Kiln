#include <basalt/server.h>
#include <basalt/server/core.hpp>
#include <basalt/server/server.h>
#include <basalt/server/values.hpp>
// Helpers over the server area: permissions, dynamic commands, broadcast, plugin files and storage, services and the
// scheduler completions, on top of wrappers/server.h. No heap, no exceptions.
#pragma once

namespace basalt::server::server {
using namespace basalt::server::core;
using namespace basalt::server::values;

/// An answer value in inline storage: `value` reads it once the call that filled it returned 0.
template <int N = BS_SERVER_BUFFER>
struct Reply {
    char data[N];
    Value value;
};

namespace impl {
template <typename Call>
int fill_reply(Reply<>& reply, Call call) {
    bs_val raw;
    const int rc = call(reply.data, static_cast<int>(sizeof reply.data), &raw);
    reply.value = Value{raw};
    return rc;
}
} // namespace impl

constexpr int kBroadcastPlayers = BS_BROADCAST_PLAYERS;
constexpr int kBroadcastConsole = BS_BROADCAST_CONSOLE;
constexpr int kProvideCheck = BS_PROVIDE_CHECK;
constexpr int kProvideMeta = BS_PROVIDE_META;

inline int permission_check(const char* subject, const char* node) { return bs_permission_check(subject, node); }

inline int permission_has(Player player, const char* node) { return bs_permission_has(player, node); }

inline int permission_grant(Player player, const char* node) { return bs_permission_set(player, node, 1); }

inline int permission_deny(Player player, const char* node) { return bs_permission_set(player, node, 0); }

inline int permission_drop(Player player, const char* node) { return bs_permission_set(player, node, -1); }

inline Text<128> permission_meta(const char* subject, const char* key) {
    return core::detail::fill<128>([&](char* out, int cap) { return bs_permission_meta(subject, key, out, cap); });
}

inline int permission_provide(int priority, int kinds, Handler handler) {
    return bs_permission_provide(priority, kinds, handler);
}

inline int permission_answer(Ctx ctx, int answer) { return basalt_permission_answer(ctx, answer); }

inline int permission_answer_meta(Ctx ctx, const char* text) {
    return basalt_permission_answer_meta(ctx, text, bs_srv_len(text));
}

inline Text<128> permission_query(Ctx ctx, const char* field) {
    Text<128> text;
    char buffer[BS_SERVER_BUFFER];
    const char* data = nullptr;
    const int length = bs_permission_query(ctx, field, buffer, sizeof buffer, &data);
    for (int i = 0; length > 0 && i < length && i < 128; ++i) {
        text.data[text.length++] = data[i];
    }
    if (length < 0) {
        text.length = length;
    }
    return text;
}

inline int command(const char* spec, const char* permission, Handler handler, Handler complete = nullptr) {
    return bs_command_ex(spec, permission, handler, complete);
}

inline int command_alias(const char* name, const char* alias) { return bs_command_alias(name, alias); }

inline int command_unregister(const char* name) { return bs_command_unregister(name); }

inline int command_run(const char* line, Player player = 0) { return bs_command_run(line, player); }

inline Text<256> complete_input(Ctx ctx) {
    return core::detail::fill<256>([&](char* out, int cap) { return bs_complete_input(ctx, out, cap); });
}

inline int complete_add(Ctx ctx, const char* candidate) { return bs_complete_add(ctx, candidate); }

inline int command_arg_f64(Ctx ctx, const char* name, double* out) { return bs_command_arg_f64(ctx, name, out); }

inline int command_arg_bool(Ctx ctx, const char* name, int* out) { return bs_command_arg_bool(ctx, name, out); }

inline int broadcast(const char* text, const Args* args, int flags) {
    return bs_broadcast(text, core::detail::raw(args), flags);
}

inline Text<256> placeholder_fill(const char* text, Player player = 0) {
    return core::detail::fill<256>([&](char* out, int cap) { return bs_placeholder_fill(text, player, out, cap); });
}

inline int info(Reply<>& reply) {
    return impl::fill_reply(reply,
                            [](char* out, int cap, bs_val* raw) { return bs_server_info(out, cap, nullptr, 0, raw); });
}

inline int call(const char* op, const ValueWriter* args, Reply<>& reply) {
    return impl::fill_reply(reply, [&](char* out, int cap, bs_val* raw) {
        return bs_server_call(op, args == nullptr ? nullptr : args->data(), args == nullptr ? 0 : args->finish(), out,
                              cap, nullptr, 0, raw);
    });
}

inline int profile_lookup(const char* name, Handler handler) { return bs_profile_lookup(name, handler); }

inline int result(Ctx ctx, Reply<>& reply) {
    return impl::fill_reply(reply, [&](char* out, int cap, bs_val* raw) { return bs_result(ctx, out, cap, raw); });
}

inline int file_read(const char* path, char* out, int cap) { return bs_file_read(path, out, cap); }

inline int file_write(const char* path, const char* data) { return bs_file_write(path, data, bs_srv_len(data)); }

inline bool file_exists(const char* path) { return bs_file_exists(path) == 1; }

inline int file_delete(const char* path, bool recursive = false) { return bs_file_delete(path, recursive ? 1 : 0); }

inline int file_list(const char* prefix, Reply<>& reply) {
    return impl::fill_reply(reply,
                            [&](char* out, int cap, bs_val* raw) { return bs_file_list(prefix, out, cap, raw); });
}

inline int file_stat(const char* path, Reply<>& reply) {
    return impl::fill_reply(reply, [&](char* out, int cap, bs_val* raw) { return bs_file_stat(path, out, cap, raw); });
}

inline int file_walk(const char* path, Reply<>& reply) {
    return impl::fill_reply(reply, [&](char* out, int cap, bs_val* raw) { return bs_file_walk(path, out, cap, raw); });
}

inline int store_set(const char* key, long long value) { return bs_store_set_i64(key, value); }

inline int store_set(const char* key, const char* value) { return bs_store_set_str(key, value); }

inline int store_get(const char* key, Reply<>& reply) {
    return impl::fill_reply(reply, [&](char* out, int cap, bs_val* raw) { return bs_store_get(key, out, cap, raw); });
}

inline int data_write(const char* name, const ValueWriter& doc) {
    return bs_data_write(name, doc.data(), doc.finish());
}

inline int data_read(const char* name, Reply<>& reply) {
    return impl::fill_reply(reply, [&](char* out, int cap, bs_val* raw) { return bs_data_read(name, out, cap, raw); });
}

inline int config_value(const char* key, Reply<>& reply) {
    return impl::fill_reply(reply,
                            [&](char* out, int cap, bs_val* raw) { return bs_config_value(key, out, cap, raw); });
}

inline int service_register(const char* name, Handler handler) { return bs_service_register(name, handler); }

inline int service_reply(Ctx ctx, const ValueWriter& doc) { return bs_service_reply(ctx, doc.data(), doc.finish()); }

inline int service_reply_null(Ctx ctx) { return bs_service_reply(ctx, nullptr, 0); }

inline bool service_has(const char* name) { return bs_service_has(name) == 1; }

inline int service_call(const char* name, const ValueWriter& args, int timeout, Handler handler) {
    return bs_service_call(name, args.data(), args.finish(), timeout, handler);
}

inline int wait(int ticks, Handler handler) { return bs_wait(ticks, handler); }

inline int at(const char* world, double x, double z, Handler handler) { return bs_at(world, x, z, handler); }

} // namespace basalt::server::server
