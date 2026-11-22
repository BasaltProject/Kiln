#include "runtime.h"

#include "basalt/server/data.h"
#include "basalt/server/menus.h"
#include "basalt/server/players.h"

int bs_particle(bs_player player, const char *id, int count, double spread) {
    return basalt_player_particle(player, id, bs_length(id), count, spread);
}

int bs_sound(bs_player player, const char *id, float volume, float pitch) {
    return basalt_player_sound(player, id, bs_length(id), volume, pitch);
}

int bs_store_top(const char *key, int limit, char *out, int cap) {
    return basalt_store_top(key, bs_length(key), limit, out, cap);
}

int bs_placeholder(const char *name, bs_handler handler) {
    const int token = bs_token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID : basalt_placeholder_register(name, bs_length(name), token);
}

bs_player bs_placeholder_player(bs_ctx ctx) { return basalt_placeholder_player(ctx); }

int bs_placeholder_result(bs_ctx ctx, const char *text) {
    return basalt_placeholder_result(ctx, text, bs_length(text));
}

int bs_menu_create(const char *title, int rows) { return basalt_menu_create(title, bs_length(title), rows); }

int bs_menu_set(int menu, int slot, const char *item, const char *name, const char *lore, bs_handler on_click) {
    const int token = bs_token_for(on_click);
    return token < 0 ? BASALT_ERR_INVALID
                     : basalt_menu_set(menu, slot, item, bs_length(item), name, bs_length(name), lore,
                                       bs_length(lore), token);
}

int bs_menu_open(int menu, bs_player player) { return basalt_menu_open(menu, player); }
bs_player bs_menu_click_player(bs_ctx ctx) { return basalt_menu_click_player(ctx); }
