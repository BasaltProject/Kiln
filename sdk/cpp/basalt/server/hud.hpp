#include <basalt/server.h>
#include <basalt/server/core.hpp>
#include <basalt/server/hud.h>
#include <basalt/server/values.hpp>
// HUD area (abi/areas/hud.h) over wrappers/hud.h. Specs are encoded values built with the Value of values.hpp; an `out`
// buffer receives an encoded value (UTF-8 text for the team and display lookups).
#pragma once

namespace basalt::server::hud {
using namespace basalt::server::core;
using namespace basalt::server::values;

inline int scoreboard_main() { return bs_hud_scoreboard_main(); }

inline int scoreboard_create() { return bs_hud_scoreboard_create(); }

inline int scoreboard_release(int board) { return bs_hud_scoreboard_release(board); }

inline int scoreboard_list(int board, const char* what, char* out, int capacity) {
    return bs_hud_scoreboard_list(board, what, out, capacity);
}

inline int objective_add(int board, const char* name, const char* spec, int length) {
    return bs_hud_objective_add(board, name, spec, length);
}

inline int objective_remove(int board, const char* name) { return bs_hud_objective_remove(board, name); }

inline int display_set(int board, const char* slot, const char* objective) {
    return bs_hud_display_set(board, slot, objective);
}

inline int display_get(int board, const char* slot, char* out, int capacity) {
    return bs_hud_display_get(board, slot, out, capacity);
}

inline int score_get(int board, const char* entry, const char* objective, int* out) {
    return bs_hud_score_get(board, entry, objective, out);
}

inline int score_set(int board, const char* entry, const char* objective, int value) {
    return bs_hud_score_set(board, entry, objective, value);
}

inline int score_reset(int board, const char* entry, const char* objective) {
    return bs_hud_score_reset(board, entry, objective);
}

inline int team_add(int board, const char* name) { return bs_hud_team_add(board, name); }

inline int team_set(int board, const char* name, const char* spec, int length) {
    return bs_hud_team_set(board, name, spec, length);
}

inline int team_remove(int board, const char* name) { return bs_hud_team_remove(board, name); }

inline int team_join(int board, const char* team, const char* entry) { return bs_hud_team_join(board, team, entry); }

inline int team_leave(int board, const char* team, const char* entry) { return bs_hud_team_leave(board, team, entry); }

inline int team_of(int board, const char* entry, char* out, int capacity) {
    return bs_hud_team_of(board, entry, out, capacity);
}

inline int scoreboard_show(int player, int board) { return bs_hud_scoreboard_show(player, board); }

inline int scoreboard_shown(int player) { return bs_hud_scoreboard_shown(player); }

inline int bossbar_create(const char* spec, int length) { return bs_hud_bossbar_create(spec, length); }

inline int bossbar_attach(const char* id) { return bs_hud_bossbar_attach(id); }

inline int bossbar_update(int bar, const char* spec, int length) { return bs_hud_bossbar_update(bar, spec, length); }

inline int bossbar_remove(int bar) { return bs_hud_bossbar_remove(bar); }

inline int bossbar_player_add(int bar, int player) { return bs_hud_bossbar_player_add(bar, player); }

inline int bossbar_player_remove(int bar, int player) { return bs_hud_bossbar_player_remove(bar, player); }

inline int bossbar_players(int bar, char* out, int capacity) { return bs_hud_bossbar_players(bar, out, capacity); }

inline int bossbar_get(int bar, char* out, int capacity) { return bs_hud_bossbar_get(bar, out, capacity); }

inline int bossbar_kept(char* out, int capacity) { return bs_hud_bossbar_kept(out, capacity); }

inline int hologram_create(const char* world, const char* text, double x, double y, double z) {
    return bs_hud_hologram_create(world, text, x, y, z);
}

inline int hologram_set(int hologram, const char* spec, int length) {
    return bs_hud_hologram_set(hologram, spec, length);
}

inline int hologram_get(int hologram, char* out, int capacity) { return bs_hud_hologram_get(hologram, out, capacity); }

inline int hologram_remove(int hologram) { return bs_hud_hologram_remove(hologram); }

inline int npc_create(const char* spec, int length, Handler handler) {
    return bs_hud_npc_create(spec, length, handler);
}

inline int npc_set(int npc, const char* spec, int length) { return bs_hud_npc_set(npc, spec, length); }

inline int npc_get(int npc, char* out, int capacity) { return bs_hud_npc_get(npc, out, capacity); }

inline int npc_item(int npc, int slot, char* out, int capacity) { return bs_hud_npc_item(npc, slot, out, capacity); }

inline int npc_attribute(int npc, const char* name, double* out) { return bs_hud_npc_attribute(npc, name, out); }

inline int npc_animate(int npc, int action) { return bs_hud_npc_animate(npc, action); }

inline int npc_remove(int npc) { return bs_hud_npc_remove(npc); }

inline int player_set(int player, const char* spec, int length) { return bs_hud_player_set(player, spec, length); }

inline int player_get(int player, char* out, int capacity) { return bs_hud_player_get(player, out, capacity); }

inline int tab_entry(int viewer, int entity_id, int listed) { return bs_hud_tab_entry(viewer, entity_id, listed); }

inline int surface_claim(const char* surface, int priority, const char* spec, int length, Handler handler) {
    return bs_hud_surface_claim(surface, priority, spec, length, handler);
}

inline int surface_update(int claim, const char* spec, int length) {
    return bs_hud_surface_update(claim, spec, length);
}

inline int surface_release(int claim) { return bs_hud_surface_release(claim); }

inline int surface_answer(int ctx, const char* spec, int length) { return bs_hud_surface_answer(ctx, spec, length); }

} // namespace basalt::server::hud
