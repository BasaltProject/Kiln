#include <basalt/server.h>
#pragma once
#include <basalt/server/core.hpp>
#include <basalt/server/players.h>

namespace basalt::server::players {
using namespace basalt::server::core;

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

inline int particle(Player player, const char* id, int count, double spread) {
    return bs_particle(player, id, count, spread);
}

inline int sound(Player player, const char* id, float volume, float pitch) {
    return bs_sound(player, id, volume, pitch);
}
} // namespace basalt::server::players
