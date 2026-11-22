#include <basalt/proxy.h>
#pragma once
#include <basalt/proxy/core.hpp>
#include <basalt/proxy/players.h>

namespace basalt::proxy::players {
using namespace basalt::proxy::core;

inline Player player_by_name(const char* name) { return bs_player_by_name(name); }

inline int players_online(Player* out, int capacity) { return bs_players_online(out, capacity); }

inline Text<256> player_name(Player player) {
    return core::detail::fill<256>([&](char* out, int cap) { return bs_player_name(player, out, cap); });
}

inline int send(Player player, const char* text, const Args* args = nullptr) {
    return bs_send(player, text, core::detail::raw(args));
}

inline int action_bar(Player player, const char* text, const Args* args = nullptr) {
    return bs_action_bar(player, text, core::detail::raw(args));
}
} // namespace basalt::proxy::players

namespace basalt::proxy::players {

inline int event_set_str(Ctx ctx, const char* field, const char* value) {
    return bs_proxy_event_set_str(ctx, field, value);
}

inline int event_set_i64(Ctx ctx, const char* field, long long value) {
    return bs_proxy_event_set_i64(ctx, field, value);
}

inline Text<64> player_uuid(Player player) {
    return core::detail::fill<64>([&](char* out, int cap) { return bs_proxy_player_uuid(player, out, cap); });
}

inline Text<256> player_server(Player player) {
    return core::detail::fill<256>([&](char* out, int cap) { return bs_proxy_player_server(player, out, cap); });
}

inline int player_ping(Player player) { return bs_proxy_player_ping(player); }

inline Text<256> player_setting_str(Player player, const char* field) {
    return core::detail::fill<256>(
        [&](char* out, int cap) { return bs_proxy_player_setting_str(player, field, out, cap); });
}

inline int player_setting_i64(Player player, const char* field, long long* out) {
    return bs_proxy_player_setting_i64(player, field, out);
}

inline bool player_has_permission(Player player, const char* node) {
    return bs_proxy_player_has_permission(player, node) == 1;
}

inline int player_title(Player player, const char* title, const char* subtitle, int fade_in = -1, int stay = -1,
                        int fade_out = -1) {
    return bs_proxy_player_title(player, title, subtitle, fade_in, stay, fade_out);
}

inline int player_connect(Player player, const char* server) { return bs_proxy_player_connect(player, server); }

inline int player_kick(Player player, const char* reason, const Args* args = nullptr) {
    return bs_proxy_player_kick(player, reason, core::detail::raw(args));
}

inline Text<1024> servers_list() {
    return core::detail::fill<1024>([&](char* out, int cap) { return bs_proxy_servers_list(out, cap); });
}

inline Text<256> server_address(const char* name) {
    return core::detail::fill<256>([&](char* out, int cap) { return bs_proxy_server_address(name, out, cap); });
}

inline int players_on(const char* server, Player* out, int capacity) {
    return bs_proxy_players_on(server, out, capacity);
}

} // namespace basalt::proxy::players
