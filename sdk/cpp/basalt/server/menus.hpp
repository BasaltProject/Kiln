#include <basalt/server.h>
#pragma once
#include <basalt/server/core.hpp>
#include <basalt/server/menus.h>

namespace basalt::server::menus {
using namespace basalt::server::core;

inline int placeholder(const char* name, Handler handler) { return bs_placeholder(name, handler); }

inline Player placeholder_player(Ctx ctx) { return bs_placeholder_player(ctx); }

inline int placeholder_result(Ctx ctx, const char* text) { return bs_placeholder_result(ctx, text); }

inline int menu_create(const char* title, int rows) { return bs_menu_create(title, rows); }

inline int menu_set(int menu, int slot, const char* item, const char* name, const char* lore, Handler on_click) {
    return bs_menu_set(menu, slot, item, name, lore, on_click);
}

inline int menu_open(int menu, Player player) { return bs_menu_open(menu, player); }

inline Player menu_click_player(Ctx ctx) { return bs_menu_click_player(ctx); }
} // namespace basalt::server::menus
