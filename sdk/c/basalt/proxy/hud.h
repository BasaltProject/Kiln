#ifndef BASALT_PROXY_HUD_H
#define BASALT_PROXY_HUD_H

#include "basalt/proxy/core.h"
#include "basalt/proxy/values.h"
#include "areas/proxy_hud.h"

/* Proxy hud area (abi/areas/proxy_hud.h): boss bars, sidebar, tab list, tab rows, server links, chat completions. A
 * NULL optional text leaves that part unchanged; every call answers BASALT_OK or a negative BASALT_ERR_*. */

#define BS_PH_BUFFER 8192

static inline int bs_ph_len(const char *text) {
    int n = 0;
    while (text != 0 && text[n]) {
        ++n;
    }
    return n;
}

static inline int bs_ph_key_str(bs_vw *w, const char *key, const char *text) {
    if (text != 0) {
        bs_vw_key(w, key);
        bs_vw_str(w, text);
    }
    return bs_vw_ok(w);
}

static inline int bs_proxy_hud_clear_title(bs_player player, int reset) {
    return basalt_proxy_hud_clear_title(player, reset);
}

static inline int bs_proxy_hud_tab_list(bs_player player, const char *header, const char *footer) {
    char buffer[BS_PH_BUFFER];
    bs_vw w;
    bs_vw_init(&w, buffer, sizeof buffer);
    bs_vw_map(&w);
    bs_ph_key_str(&w, "header", header);
    bs_ph_key_str(&w, "footer", footer);
    bs_vw_end(&w);
    const int n = bs_vw_finish(&w);
    return n < 0 ? BASALT_ERR_INVALID : basalt_proxy_hud_tab_list(player, buffer, n);
}

static inline int bs_proxy_hud_bossbar_show(bs_player player, const char *id, const char *text, double progress,
                                            const char *color, const char *overlay) {
    char buffer[BS_PH_BUFFER];
    bs_vw w;
    bs_vw_init(&w, buffer, sizeof buffer);
    bs_vw_map(&w);
    bs_ph_key_str(&w, "text", text);
    bs_vw_key(&w, "progress");
    bs_vw_f64(&w, progress);
    bs_ph_key_str(&w, "color", color);
    bs_ph_key_str(&w, "overlay", overlay);
    bs_vw_end(&w);
    const int n = bs_vw_finish(&w);
    return n < 0 ? BASALT_ERR_INVALID : basalt_proxy_hud_bossbar_show(player, id, bs_ph_len(id), buffer, n);
}

static inline int bs_proxy_hud_bossbar_hide(bs_player player, const char *id) {
    return basalt_proxy_hud_bossbar_hide(player, id, bs_ph_len(id));
}

static inline int bs_proxy_hud_bossbar_progress(bs_player player, const char *id, double progress) {
    return basalt_proxy_hud_bossbar_progress(player, id, bs_ph_len(id), progress);
}

static inline int bs_proxy_hud_bossbar_title(bs_player player, const char *id, const char *text) {
    return basalt_proxy_hud_bossbar_title(player, id, bs_ph_len(id), text, bs_ph_len(text), 0, 0);
}

static inline int bs_proxy_hud_bossbar_style(bs_player player, const char *id, const char *color,
                                             const char *overlay) {
    return basalt_proxy_hud_bossbar_style(player, id, bs_ph_len(id), color, bs_ph_len(color), overlay,
                                          bs_ph_len(overlay));
}

static inline int bs_proxy_hud_sidebar(bs_player player, const char *title, const char *const *lines, int count) {
    char buffer[BS_PH_BUFFER];
    bs_vw w;
    bs_vw_init(&w, buffer, sizeof buffer);
    bs_vw_map(&w);
    bs_ph_key_str(&w, "title", title);
    bs_vw_key(&w, "lines");
    bs_vw_list(&w);
    for (int i = 0; i < count; ++i) {
        bs_vw_str(&w, lines[i]);
    }
    bs_vw_end(&w);
    bs_vw_end(&w);
    const int n = bs_vw_finish(&w);
    return n < 0 ? BASALT_ERR_INVALID : basalt_proxy_hud_sidebar(player, buffer, n);
}

static inline int bs_proxy_hud_sidebar_clear(bs_player player) { return basalt_proxy_hud_sidebar_clear(player); }

static inline int bs_proxy_hud_tab_entry(bs_player player, const char *id, const char *name, const char *text,
                                         int latency) {
    char buffer[BS_PH_BUFFER];
    bs_vw w;
    bs_vw_init(&w, buffer, sizeof buffer);
    bs_vw_map(&w);
    bs_ph_key_str(&w, "name", name);
    bs_ph_key_str(&w, "text", text);
    bs_vw_key(&w, "latency");
    bs_vw_i64(&w, latency);
    bs_vw_end(&w);
    const int n = bs_vw_finish(&w);
    return n < 0 ? BASALT_ERR_INVALID : basalt_proxy_hud_tab_entry(player, id, bs_ph_len(id), buffer, n);
}

static inline int bs_proxy_hud_tab_entry_remove(bs_player player, const char *id) {
    return basalt_proxy_hud_tab_entry_remove(player, id, bs_ph_len(id));
}

/* `pairs` holds `count` entries, label then url each (2 * count strings). */
static inline int bs_proxy_hud_server_links(bs_player player, const char *const *pairs, int count) {
    char buffer[BS_PH_BUFFER];
    bs_vw w;
    bs_vw_init(&w, buffer, sizeof buffer);
    bs_vw_list(&w);
    for (int i = 0; i < count; ++i) {
        bs_vw_list(&w);
        bs_vw_str(&w, pairs[2 * i]);
        bs_vw_str(&w, pairs[2 * i + 1]);
        bs_vw_end(&w);
    }
    bs_vw_end(&w);
    const int n = bs_vw_finish(&w);
    return n < 0 ? BASALT_ERR_INVALID : basalt_proxy_hud_server_links(player, buffer, n);
}

static inline int bs_proxy_hud_chat_completions(bs_player player, const char *action, const char *const *words,
                                                int count) {
    char buffer[BS_PH_BUFFER];
    bs_vw w;
    bs_vw_init(&w, buffer, sizeof buffer);
    bs_vw_list(&w);
    for (int i = 0; i < count; ++i) {
        bs_vw_str(&w, words[i]);
    }
    bs_vw_end(&w);
    const int n = bs_vw_finish(&w);
    return n < 0 ? BASALT_ERR_INVALID : basalt_proxy_hud_chat_completions(player, action, bs_ph_len(action), buffer, n);
}

#endif
