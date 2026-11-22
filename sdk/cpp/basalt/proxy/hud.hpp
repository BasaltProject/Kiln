#include <basalt/proxy.h>
#include <basalt/proxy/core.hpp>
#include <basalt/proxy/hud.h>
#include <basalt/proxy/values.hpp>
// Proxy hud area (abi/areas/proxy_hud.h) over wrappers/proxy_hud.h. A nullptr optional text leaves that part unchanged.
#pragma once

namespace basalt::proxy::hud {
using namespace basalt::proxy::core;
using namespace basalt::proxy::values;

inline int hud_clear_title(Player player, bool reset) { return bs_proxy_hud_clear_title(player, reset ? 1 : 0); }

inline int hud_tab_list(Player player, const char* header, const char* footer) {
    return bs_proxy_hud_tab_list(player, header, footer);
}

inline int hud_bossbar_show(Player player, const char* id, const char* text, double progress = 1.0,
                            const char* color = "purple", const char* overlay = "progress") {
    return bs_proxy_hud_bossbar_show(player, id, text, progress, color, overlay);
}

inline int hud_bossbar_hide(Player player, const char* id) { return bs_proxy_hud_bossbar_hide(player, id); }

inline int hud_bossbar_progress(Player player, const char* id, double progress) {
    return bs_proxy_hud_bossbar_progress(player, id, progress);
}

inline int hud_bossbar_title(Player player, const char* id, const char* text) {
    return bs_proxy_hud_bossbar_title(player, id, text);
}

inline int hud_bossbar_style(Player player, const char* id, const char* color, const char* overlay) {
    return bs_proxy_hud_bossbar_style(player, id, color, overlay);
}

inline int hud_sidebar(Player player, const char* title, const char* const* lines, int count) {
    return bs_proxy_hud_sidebar(player, title, lines, count);
}

inline int hud_sidebar_clear(Player player) { return bs_proxy_hud_sidebar_clear(player); }

inline int hud_tab_entry(Player player, const char* id, const char* name, const char* text, int latency = 0) {
    return bs_proxy_hud_tab_entry(player, id, name, text, latency);
}

inline int hud_tab_entry_remove(Player player, const char* id) { return bs_proxy_hud_tab_entry_remove(player, id); }

/// `pairs` holds `count` entries, label then url each.
inline int hud_server_links(Player player, const char* const* pairs, int count) {
    return bs_proxy_hud_server_links(player, pairs, count);
}

inline int hud_chat_completions(Player player, const char* action, const char* const* words, int count) {
    return bs_proxy_hud_chat_completions(player, action, words, count);
}

} // namespace basalt::proxy::hud
