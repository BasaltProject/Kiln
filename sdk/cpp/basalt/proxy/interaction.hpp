#include <basalt/proxy.h>
#include <basalt/proxy/core.hpp>
#include <basalt/proxy/interaction.h>
#include <basalt/proxy/values.hpp>
// Proxy interaction area (abi/areas/proxy_interaction.h) over wrappers/proxy_interaction.h. Read an observed event or a
// completion with `value_result` and the ValueReader of values.hpp.
#pragma once

namespace basalt::proxy::interaction {
using namespace basalt::proxy::core;
using namespace basalt::proxy::values;

/// Observes player.resource_pack: the value holds player, player_name, id and status.
inline int watch_resource_packs(Handler handler) { return bs_proxy_watch_resource_packs(handler); }

/// Observes player.settings: the value holds player, player_name and settings.
inline int watch_settings(Handler handler) { return bs_proxy_watch_settings(handler); }

inline int value_result(Ctx ctx, char* out, int capacity) { return bs_proxy_value_result(ctx, out, capacity); }

/// kind is "notice", "confirmation" or "multi_action"; body may be null; each button is a label and a command.
inline int dialog_show(Player player, const char* kind, const char* title, const char* body, const char* const* labels,
                       const char* const* commands, int count, bool closable = true) {
    return bs_proxy_dialog_show(player, kind, title, body, labels, commands, count, closable);
}

inline int dialog_clear(Player player) { return bs_proxy_dialog_clear(player); }

/// Pushes a pack; hash, prompt and id may be null. The pack id text goes to `out`, the result is its length.
inline int resource_pack_push(Player player, const char* url, const char* hash, bool required, const char* prompt,
                              const char* id, char* out, int capacity) {
    return bs_proxy_resource_pack_push(player, url, hash, required, prompt, id, out, capacity);
}

inline int resource_pack_remove(Player player, const char* id) { return bs_proxy_resource_pack_remove(player, id); }

inline int resource_pack_clear(Player player) { return bs_proxy_resource_pack_clear(player); }

inline int cookie_set(Player player, const char* key, const void* data, int length) {
    return bs_proxy_cookie_set(player, key, data, length);
}

/// Returns the request handle; the handler reads the cookie with cookie_result.
inline int cookie_request(Player player, const char* key, Handler handler) {
    return bs_proxy_cookie_request(player, key, handler);
}

/// The cookie length (written to `out` only when it fits), or a negative code when the client had none.
inline int cookie_result(Ctx ctx, void* out, int capacity) { return bs_proxy_cookie_result(ctx, out, capacity); }

using core::request_cancel;

inline int player_transfer(Player player, const char* host, int port = 25565) {
    return bs_proxy_player_transfer(player, host, port);
}

inline Player players_by_uuid(const char* uuid) { return bs_proxy_players_by_uuid(uuid); }

/// Players on the proxy, or on `server` when it is not null or empty.
inline int players_count(const char* server = "") { return bs_proxy_players_count(server); }

inline int players_broadcast(const char* text, const Args* args = nullptr) {
    return bs_proxy_players_broadcast(text, core::detail::raw(args));
}

inline Text<64> player_address(Player player) {
    return core::detail::fill<64>([&](char* out, int cap) { return bs_proxy_player_address(player, out, cap); });
}

inline int player_port(Player player) { return bs_proxy_player_port(player); }

inline int player_settings(Player player, char* out, int capacity) {
    return bs_proxy_player_settings(player, out, capacity);
}

inline double player_data_get_f64(Player player, const char* key, double fallback) {
    return bs_proxy_player_data_get_f64(player, key, fallback);
}

inline int player_data_set_f64(Player player, const char* key, double value) {
    return bs_proxy_player_data_set_f64(player, key, value);
}

} // namespace basalt::proxy::interaction
