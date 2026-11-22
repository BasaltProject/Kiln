#ifndef BASALT_SERVER_HUD_H
#define BASALT_SERVER_HUD_H

#include "basalt/server/core.h"
#include "basalt/server/values.h"
#include "areas/hud.h"

/* HUD area (abi/areas/hud.h): scoreboards, boss bars, holograms, NPCs, tab and chat surfaces. Specs are encoded
 * values built with bs_value_* of values.h; the buffers an `out` fills hold an encoded value (UTF-8 text for the
 * team and display lookups) and the result is the length the full answer needs, or a negative code. */

static inline int bs_hud_length(const char *text) {
    int n = 0;
    while (text != 0 && text[n]) {
        ++n;
    }
    return n;
}

static inline int bs_hud_scoreboard_main(void) {
    return basalt_hud_scoreboard_main();
}

static inline int bs_hud_scoreboard_create(void) {
    return basalt_hud_scoreboard_create();
}

static inline int bs_hud_scoreboard_release(int board) {
    return basalt_hud_scoreboard_release(board);
}

static inline int bs_hud_scoreboard_list(int board, const char *what, char *out, int cap) {
    return basalt_hud_scoreboard_list(board, what, bs_hud_length(what), out, cap);
}

static inline int bs_hud_objective_add(int board, const char *name, const char *spec, int length) {
    return basalt_hud_objective_add(board, name, bs_hud_length(name), spec, length);
}

static inline int bs_hud_objective_remove(int board, const char *name) {
    return basalt_hud_objective_remove(board, name, bs_hud_length(name));
}

static inline int bs_hud_display_set(int board, const char *slot, const char *objective) {
    return basalt_hud_display_set(board, slot, bs_hud_length(slot), objective, bs_hud_length(objective));
}

static inline int bs_hud_display_get(int board, const char *slot, char *out, int cap) {
    return basalt_hud_display_get(board, slot, bs_hud_length(slot), out, cap);
}

static inline int bs_hud_score_get(int board, const char *entry, const char *objective, int32_t *out) {
    return basalt_hud_score_get(board, entry, bs_hud_length(entry), objective, bs_hud_length(objective), out);
}

static inline int bs_hud_score_set(int board, const char *entry, const char *objective, int value) {
    return basalt_hud_score_set(board, entry, bs_hud_length(entry), objective, bs_hud_length(objective), value);
}

static inline int bs_hud_score_reset(int board, const char *entry, const char *objective) {
    return basalt_hud_score_reset(board, entry, bs_hud_length(entry), objective, bs_hud_length(objective));
}

static inline int bs_hud_team_add(int board, const char *name) {
    return basalt_hud_team_add(board, name, bs_hud_length(name));
}

static inline int bs_hud_team_set(int board, const char *name, const char *spec, int length) {
    return basalt_hud_team_set(board, name, bs_hud_length(name), spec, length);
}

static inline int bs_hud_team_remove(int board, const char *name) {
    return basalt_hud_team_remove(board, name, bs_hud_length(name));
}

static inline int bs_hud_team_join(int board, const char *team, const char *entry) {
    return basalt_hud_team_join(board, team, bs_hud_length(team), entry, bs_hud_length(entry));
}

static inline int bs_hud_team_leave(int board, const char *team, const char *entry) {
    return basalt_hud_team_leave(board, team, bs_hud_length(team), entry, bs_hud_length(entry));
}

static inline int bs_hud_team_of(int board, const char *entry, char *out, int cap) {
    return basalt_hud_team_of(board, entry, bs_hud_length(entry), out, cap);
}

static inline int bs_hud_scoreboard_show(int player, int board) {
    return basalt_hud_scoreboard_show(player, board);
}

static inline int bs_hud_scoreboard_shown(int player) {
    return basalt_hud_scoreboard_shown(player);
}

static inline int bs_hud_bossbar_create(const char *spec, int length) {
    return basalt_hud_bossbar_create(spec, length);
}

static inline int bs_hud_bossbar_attach(const char *id) {
    return basalt_hud_bossbar_attach(id, bs_hud_length(id));
}

static inline int bs_hud_bossbar_update(int bar, const char *spec, int length) {
    return basalt_hud_bossbar_update(bar, spec, length);
}

static inline int bs_hud_bossbar_remove(int bar) {
    return basalt_hud_bossbar_remove(bar);
}

static inline int bs_hud_bossbar_player_add(int bar, int player) {
    return basalt_hud_bossbar_player_add(bar, player);
}

static inline int bs_hud_bossbar_player_remove(int bar, int player) {
    return basalt_hud_bossbar_player_remove(bar, player);
}

static inline int bs_hud_bossbar_players(int bar, char *out, int cap) {
    return basalt_hud_bossbar_players(bar, out, cap);
}

static inline int bs_hud_bossbar_get(int bar, char *out, int cap) {
    return basalt_hud_bossbar_get(bar, out, cap);
}

static inline int bs_hud_bossbar_kept(char *out, int cap) {
    return basalt_hud_bossbar_kept(out, cap);
}

static inline int bs_hud_hologram_create(const char *world, const char *text, double x, double y, double z) {
    return basalt_hud_hologram_create(world, bs_hud_length(world), text, bs_hud_length(text), x, y, z);
}

static inline int bs_hud_hologram_set(int hologram, const char *spec, int length) {
    return basalt_hud_hologram_set(hologram, spec, length);
}

static inline int bs_hud_hologram_get(int hologram, char *out, int cap) {
    return basalt_hud_hologram_get(hologram, out, cap);
}

static inline int bs_hud_hologram_remove(int hologram) {
    return basalt_hud_hologram_remove(hologram);
}

static inline int bs_hud_npc_create(const char *spec, int length, bs_handler handler) {
    const int token = bs_token_for(handler);
    if (token < 0) {
        return BASALT_ERR_INVALID;
    }
    return basalt_hud_npc_create(spec, length, token);
}

static inline int bs_hud_npc_set(int npc, const char *spec, int length) {
    return basalt_hud_npc_set(npc, spec, length);
}

static inline int bs_hud_npc_get(int npc, char *out, int cap) {
    return basalt_hud_npc_get(npc, out, cap);
}

static inline int bs_hud_npc_item(int npc, int slot, char *out, int cap) {
    return basalt_hud_npc_item(npc, slot, out, cap);
}

static inline int bs_hud_npc_attribute(int npc, const char *name, double *out) {
    return basalt_hud_npc_attribute(npc, name, bs_hud_length(name), out);
}

static inline int bs_hud_npc_animate(int npc, int action) {
    return basalt_hud_npc_animate(npc, action);
}

static inline int bs_hud_npc_remove(int npc) {
    return basalt_hud_npc_remove(npc);
}

static inline int bs_hud_player_set(int player, const char *spec, int length) {
    return basalt_hud_player_set(player, spec, length);
}

static inline int bs_hud_player_get(int player, char *out, int cap) {
    return basalt_hud_player_get(player, out, cap);
}

static inline int bs_hud_tab_entry(int viewer, int entity_id, int listed) {
    return basalt_hud_tab_entry(viewer, entity_id, listed);
}

static inline int bs_hud_surface_claim(const char *surface, int priority, const char *spec, int length, bs_handler handler) {
    const int token = bs_token_for(handler);
    if (token < 0) {
        return BASALT_ERR_INVALID;
    }
    return basalt_hud_surface_claim(surface, bs_hud_length(surface), priority, spec, length, token);
}

static inline int bs_hud_surface_update(int claim, const char *spec, int length) {
    return basalt_hud_surface_update(claim, spec, length);
}

static inline int bs_hud_surface_release(int claim) {
    return basalt_hud_surface_release(claim);
}

static inline int bs_hud_surface_answer(int ctx, const char *spec, int length) {
    return basalt_hud_surface_answer(ctx, spec, length);
}

#endif
