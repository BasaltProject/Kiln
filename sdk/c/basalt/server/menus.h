/* Chest menus, placeholders, forms and dialogs. */
#ifndef BASALT_SERVER_MENUS_H
#define BASALT_SERVER_MENUS_H

#include "basalt/server/events.h"

#ifdef __cplusplus
extern "C" {
#endif

int bs_placeholder(const char *name, bs_handler handler);
bs_player bs_placeholder_player(bs_ctx ctx);
int bs_placeholder_result(bs_ctx ctx, const char *text);

int bs_menu_create(const char *title, int rows);
int bs_menu_set(int menu, int slot, const char *item, const char *name, const char *lore, bs_handler on_click);
int bs_menu_open(int menu, bs_player player);
bs_player bs_menu_click_player(bs_ctx ctx);

#ifdef __cplusplus
}
#endif

static inline int32_t bs_ev_value_result_fn(void *user, char *out, int32_t cap) {
    return basalt_value_result(((const bs_ev_call *)user)->a, out, cap);
}

/* Shows a form (a finished document); `handler` runs once with the answer, read it with bs_completion_value. */
static inline int bs_form_open(bs_player player, const void *form, int form_len, bs_handler handler) {
    const int token = bs_token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID : basalt_form_open(player, (const char *)form, form_len, token);
}

static inline int bs_form_close(bs_player player) { return basalt_form_close(player); }

/* Shows a dialog (SNBT or a registry id); `handler` runs once on the first custom click. */
static inline int bs_dialog_show(bs_player player, const char *dialog, bs_handler handler) {
    const int token = bs_token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID : basalt_dialog_show(player, dialog, bs_ev_len(dialog), token);
}

static inline int bs_dialog_clear(bs_player player) { return basalt_dialog_clear(player); }

/* The value a form or dialog completion carries, inside its handler. */
static inline int bs_completion_value(bs_ctx ctx, char *buf, int cap, bs_val *root) {
    bs_ev_call c = {ctx, 0, 0, 0, 0};
    return bs_out_value(bs_ev_value_result_fn, &c, buf, cap, 0, 0, root);
}

#endif
