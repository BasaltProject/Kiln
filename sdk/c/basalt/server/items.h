/* Helpers over the items area: build an item stack value, move stacks in and out of an inventory, query the registry.
 * Header only, no libc, no heap. Every function returns the import status (negative on error) unless it says more. */
#ifndef BASALT_SERVER_ITEMS_H
#define BASALT_SERVER_ITEMS_H

#include "basalt/server/core.h"
#include "basalt/server/values.h"
#include "areas/items.h"

#define BS_ITEM_BUFFER 512

/* Writes `{id, count[, name]}` into the writer; `name` may be NULL. */
static inline void bs_item_write(bs_vw *w, const char *id, int32_t count, const char *name) {
    bs_vw_map(w);
    bs_vw_key(w, "id");
    bs_vw_str(w, id);
    bs_vw_key(w, "count");
    bs_vw_i64(w, count);
    if (name != 0) {
        bs_vw_key(w, "name");
        bs_vw_str(w, name);
    }
    bs_vw_end(w);
}

/* The length of the item document written to `buffer`, or a negative error. */
static inline int32_t bs_item_stack(void *buffer, int32_t cap, const char *id, int32_t count, const char *name) {
    bs_vw w;
    bs_vw_init(&w, buffer, cap);
    bs_item_write(&w, id, count, name);
    return bs_vw_finish(&w);
}

static inline int32_t bs_text_len(const char *text) {
    int32_t n = 0;
    while (text[n] != 0) {
        n++;
    }
    return n;
}

/* Gives `count` of `id` to the inventory; returns how many did NOT fit, or a negative error. */
static inline int32_t bs_inventory_give(int32_t inventory, const char *id, int32_t count, const char *name) {
    unsigned char buffer[BS_ITEM_BUFFER];
    const int32_t n = bs_item_stack(buffer, BS_ITEM_BUFFER, id, count, name);
    return n < 0 ? n : basalt_inventory_add(inventory, (const char *)buffer, n);
}

static inline int32_t bs_inventory_put(int32_t inventory, int32_t slot, const char *id, int32_t count) {
    unsigned char buffer[BS_ITEM_BUFFER];
    const int32_t n = bs_item_stack(buffer, BS_ITEM_BUFFER, id, count, 0);
    return n < 0 ? n : basalt_inventory_set(inventory, slot, (const char *)buffer, n);
}

/* Removes up to `count` of `id`; returns how many were removed. */
static inline int32_t bs_inventory_take(int32_t inventory, const char *id, int32_t count) {
    return basalt_inventory_remove(inventory, id, bs_text_len(id), count);
}

static inline int32_t bs_inventory_has(int32_t inventory, const char *id) {
    return basalt_inventory_count(inventory, id, bs_text_len(id));
}

/* The count of the stack in `slot`, 0 for an empty slot, negative for an error. */
static inline int32_t bs_inventory_slot_count(int32_t inventory, int32_t slot) {
    unsigned char buffer[BS_ITEM_BUFFER];
    const int32_t n = basalt_inventory_get(inventory, slot, (char *)buffer, BS_ITEM_BUFFER);
    bs_val root;
    int64_t count = 0;
    if (n < 0) {
        return n;
    }
    if (n > BS_ITEM_BUFFER || bs_value_open(&root, buffer, n) != 0) {
        return BASALT_ERR_INVALID;
    }
    if (!bs_value_is_null(root)) {
        bs_value_as_i64(bs_value_find(root, "count"), &count);
    }
    return (int32_t)count;
}

/* Puts the stack in a menu slot; `on_click` runs when a player clicks it (0 for none). */
static inline int32_t bs_menu_slot(int32_t menu, int32_t slot, const char *id, int32_t count, const char *name,
                                   bs_handler on_click) {
    unsigned char buffer[BS_ITEM_BUFFER];
    const int32_t n = bs_item_stack(buffer, BS_ITEM_BUFFER, id, count, name);
    const int32_t token = bs_token_for(on_click);
    return n < 0 ? n : token < 0 ? BASALT_ERR_INVALID : basalt_container_menu_set(menu, slot, (const char *)buffer, n, token);
}

static inline int32_t bs_registry(const char *kind, const char *key, char *out, int32_t cap) {
    return basalt_registry_get(kind, bs_text_len(kind), key, bs_text_len(key), out, cap);
}

static inline int32_t bs_cooldown(int32_t player, const char *id) {
    return basalt_item_cooldown(player, id, bs_text_len(id));
}

static inline int32_t bs_set_cooldown(int32_t player, const char *id, int32_t ticks) {
    return basalt_item_set_cooldown(player, id, bs_text_len(id), ticks);
}

static inline int32_t bs_recipe_known(const char *id) { return basalt_recipe_has(id, bs_text_len(id)); }

#endif
