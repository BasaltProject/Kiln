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

/* ---- Recipes, windows, container menus, loot and registries ----
 * A structured argument is a finished value document (use bs_vw) given as pointer and length. The functions that
 * answer a value take a buffer and open it as `root`; an answer bigger than the buffer is BASALT_ERR_INVALID, so pass
 * a bigger buffer for the big ones (bs_recipe_ids, bs_loot_tables, bs_registry_get of a list). */

enum { BS_IT_RECIPE_IDS, BS_IT_RECIPE_GET, BS_IT_RECIPE_STATION, BS_IT_RECIPE_BOOK, BS_IT_WINDOW_INFO,
       BS_IT_CLICK_INFO, BS_IT_LOOT_TABLES, BS_IT_LOOT_ROLL, BS_IT_LOOT_ROLL_JSON, BS_IT_LOOT_NUMBER,
       BS_IT_LOOT_APPLY, BS_IT_LOOT_ENCHANT, BS_IT_TRADES_ROLL, BS_IT_REGISTRY };

typedef struct {
    int32_t which;
    const char *a;
    int32_t alen;
    const char *b;
    int32_t blen;
    int32_t number;
    int64_t seed;
} bs_it_call;

/* A missing optional document is the null value. */
static inline const char *bs_it_doc(const void *doc, int32_t *len) {
    static const char null_doc[1] = {(char)BASALT_VALUE_NULL};
    if (doc == 0) {
        *len = 1;
        return null_doc;
    }
    return (const char *)doc;
}

static inline int32_t bs_it_dispatch(void *user, char *out, int32_t cap) {
    const bs_it_call *c = (const bs_it_call *)user;
    switch (c->which) {
    case BS_IT_RECIPE_IDS: return basalt_recipe_ids(out, cap);
    case BS_IT_RECIPE_GET: return basalt_recipe_get(c->a, c->alen, out, cap);
    case BS_IT_RECIPE_STATION: return basalt_recipe_station(c->a, c->alen, c->b, c->blen, out, cap);
    case BS_IT_RECIPE_BOOK: return basalt_recipe_book_get(c->number, out, cap);
    case BS_IT_WINDOW_INFO: return basalt_window_info(c->number, out, cap);
    case BS_IT_CLICK_INFO: return basalt_container_menu_click_info(c->number, out, cap);
    case BS_IT_LOOT_TABLES: return basalt_loot_tables(out, cap);
    case BS_IT_LOOT_ROLL: return basalt_loot_roll(c->a, c->alen, c->b, c->blen, out, cap);
    case BS_IT_LOOT_ROLL_JSON: return basalt_loot_roll_json(c->a, c->alen, c->b, c->blen, out, cap);
    case BS_IT_LOOT_NUMBER: return basalt_loot_number(c->a, c->alen, c->number, c->seed, out, cap);
    case BS_IT_LOOT_APPLY: return basalt_loot_apply(c->a, c->alen, out, cap);
    case BS_IT_LOOT_ENCHANT: return basalt_loot_enchant(c->a, c->alen, out, cap);
    case BS_IT_TRADES_ROLL: return basalt_trades_roll(c->a, c->alen, c->number, c->b, c->blen, out, cap);
    case BS_IT_REGISTRY: return basalt_registry_get(c->a, c->alen, c->b, c->blen, out, cap);
    default: return BASALT_ERR_INVALID;
    }
}

static inline int32_t bs_it_value(bs_it_call *c, char *buf, int32_t cap, bs_val *root) {
    return bs_out_value(bs_it_dispatch, c, buf, cap, 0, 0, root);
}

/* Recipes. `json` is the datapack recipe JSON text. */
static inline int32_t bs_recipe_add(const char *id, const char *json) {
    return basalt_recipe_add(id, bs_text_len(id), json, bs_text_len(json));
}

static inline int32_t bs_recipe_remove(const char *id) { return basalt_recipe_remove(id, bs_text_len(id)); }

/* 1 when a recipe by that id exists, else 0. */
static inline int32_t bs_recipe_has(const char *id) { return basalt_recipe_has(id, bs_text_len(id)); }

/* Every recipe id, a list of strings. */
static inline int32_t bs_recipe_ids(char *buf, int32_t cap, bs_val *root) {
    bs_it_call c = {BS_IT_RECIPE_IDS, 0, 0, 0, 0, 0, 0};
    return bs_it_value(&c, buf, cap, root);
}

static inline int32_t bs_recipe_clear(void) { return basalt_recipe_clear(); }

static inline int32_t bs_recipe_reset(void) { return basalt_recipe_reset(); }

static inline int32_t bs_recipe_reset_brewing(void) { return basalt_recipe_reset_brewing(); }

/* A recipe as a map of its fields. */
static inline int32_t bs_recipe_get(const char *id, char *buf, int32_t cap, bs_val *root) {
    bs_it_call c = {BS_IT_RECIPE_GET, id, bs_text_len(id), 0, 0, 0, 0};
    return bs_it_value(&c, buf, cap, root);
}

/* What a crafting station makes: `kind` is "crafting", "anvil" or "smithing", `input` its document. */
static inline int32_t bs_recipe_station(const char *kind, const void *input, int32_t input_len, char *buf, int32_t cap,
                                        bs_val *root) {
    bs_it_call c = {BS_IT_RECIPE_STATION, kind, bs_text_len(kind), (const char *)input, input_len, 0, 0};
    return input_len < 0 ? BASALT_ERR_INVALID : bs_it_value(&c, buf, cap, root);
}

/* The recipe book: the ids a player discovered, a list of strings. */
static inline int32_t bs_recipe_book_get(bs_player player, char *buf, int32_t cap, bs_val *root) {
    bs_it_call c = {BS_IT_RECIPE_BOOK, 0, 0, 0, 0, player, 0};
    return bs_it_value(&c, buf, cap, root);
}

/* `ids` is a list document of recipe ids; learn 1 discovers them, 0 forgets them. Returns how many changed. */
static inline int32_t bs_recipe_book_set(bs_player player, const void *ids, int32_t ids_len, int32_t learn) {
    return ids_len < 0 ? BASALT_ERR_INVALID : basalt_recipe_book_set(player, (const char *)ids, ids_len, learn);
}

static inline int32_t bs_recipe_book_has(bs_player player, const char *id) {
    return basalt_recipe_book_has(player, id, bs_text_len(id));
}

static inline int32_t bs_recipe_book_resend(bs_player player) { return basalt_recipe_book_resend(player); }

/* Merchants and windows. */
static inline int32_t bs_merchant_open(bs_player player, const void *spec, int32_t spec_len) {
    return spec_len < 0 ? BASALT_ERR_INVALID : basalt_merchant_open(player, (const char *)spec, spec_len);
}

/* Returns the container id (positive). */
static inline int32_t bs_window_open(bs_player player, const void *spec, int32_t spec_len) {
    return spec_len < 0 ? BASALT_ERR_INVALID : basalt_window_open(player, (const char *)spec, spec_len);
}

static inline int32_t bs_window_info(bs_player player, char *buf, int32_t cap, bs_val *root) {
    bs_it_call c = {BS_IT_WINDOW_INFO, 0, 0, 0, 0, player, 0};
    return bs_it_value(&c, buf, cap, root);
}

static inline int32_t bs_window_close(bs_player player) { return basalt_window_close(player); }

static inline int32_t bs_window_update(bs_player player, const void *spec, int32_t spec_len) {
    return spec_len < 0 ? BASALT_ERR_INVALID : basalt_window_update(player, (const char *)spec, spec_len);
}

/* Container menus. */
static inline int32_t bs_container_menu_create(const void *spec, int32_t spec_len) {
    return spec_len < 0 ? BASALT_ERR_INVALID : basalt_container_menu_create((const char *)spec, spec_len);
}

/* Puts the item document in a slot; `on_click` runs when a player clicks it (0 for none). A NULL item clears it. */
static inline int32_t bs_container_menu_set(int32_t menu, int32_t slot, const void *item, int32_t item_len,
                                            bs_handler on_click) {
    int32_t len = item_len;
    const char *doc = bs_it_doc(item, &len);
    const int32_t token = bs_token_for(on_click);
    return len < 0 || token < 0 ? BASALT_ERR_INVALID : basalt_container_menu_set(menu, slot, doc, len, token);
}

static inline int32_t bs_container_menu_clear(int32_t menu) { return basalt_container_menu_clear(menu); }

static inline int32_t bs_container_menu_open(int32_t menu, bs_player player) {
    return basalt_container_menu_open(menu, player);
}

static inline int32_t bs_container_menu_close(int32_t menu, bs_player player) {
    return basalt_container_menu_close(menu, player);
}

/* Inside a click handler: {player, slot, button, mode}. */
static inline int32_t bs_container_menu_click_info(bs_ctx ctx, char *buf, int32_t cap, bs_val *root) {
    bs_it_call c = {BS_IT_CLICK_INFO, 0, 0, 0, 0, ctx, 0};
    return bs_it_value(&c, buf, cap, root);
}

/* Inside a click handler: replaces the stack shown in the clicked slot; a NULL item clears it. */
static inline int32_t bs_container_menu_click_result(bs_ctx ctx, const void *item, int32_t item_len) {
    int32_t len = item_len;
    const char *doc = bs_it_doc(item, &len);
    return len < 0 ? BASALT_ERR_INVALID : basalt_container_menu_click_result(ctx, doc, len);
}

static inline int32_t bs_container_menu_click_cancel(bs_ctx ctx, int32_t cancelled) {
    return basalt_container_menu_click_cancel(ctx, cancelled);
}

/* Loot and trades. A roll `spec` is a document (world, x, y, z, luck, seed, looter), or NULL for the defaults. */
static inline int32_t bs_loot_tables(char *buf, int32_t cap, bs_val *root) {
    bs_it_call c = {BS_IT_LOOT_TABLES, 0, 0, 0, 0, 0, 0};
    return bs_it_value(&c, buf, cap, root);
}

static inline int32_t bs_loot_roll(const char *table, const void *spec, int32_t spec_len, char *buf, int32_t cap,
                                   bs_val *root) {
    int32_t len = spec_len;
    const char *doc = bs_it_doc(spec, &len);
    bs_it_call c = {BS_IT_LOOT_ROLL, table, bs_text_len(table), doc, len, 0, 0};
    return len < 0 ? BASALT_ERR_INVALID : bs_it_value(&c, buf, cap, root);
}

/* The same for a loot table given as JSON text. */
static inline int32_t bs_loot_roll_json(const char *json, const void *spec, int32_t spec_len, char *buf, int32_t cap,
                                        bs_val *root) {
    int32_t len = spec_len;
    const char *doc = bs_it_doc(spec, &len);
    bs_it_call c = {BS_IT_LOOT_ROLL_JSON, json, bs_text_len(json), doc, len, 0, 0};
    return len < 0 ? BASALT_ERR_INVALID : bs_it_value(&c, buf, cap, root);
}

/* Evaluates a number provider given as JSON text; as_float 1 picks a float provider, 0 an int one. */
static inline int32_t bs_loot_number(const char *json, int32_t as_float, int64_t seed, char *buf, int32_t cap,
                                     bs_val *root) {
    bs_it_call c = {BS_IT_LOOT_NUMBER, json, bs_text_len(json), 0, 0, as_float, seed};
    return bs_it_value(&c, buf, cap, root);
}

/* `spec` is {functions: JSON text, item, world, x, y, z, seed}; the answer is the resulting item. */
static inline int32_t bs_loot_apply(const void *spec, int32_t spec_len, char *buf, int32_t cap, bs_val *root) {
    bs_it_call c = {BS_IT_LOOT_APPLY, (const char *)spec, spec_len, 0, 0, 0, 0};
    return spec_len < 0 ? BASALT_ERR_INVALID : bs_it_value(&c, buf, cap, root);
}

/* `spec` is {item, levels, options, seed, world}; the answer is the resulting item. */
static inline int32_t bs_loot_enchant(const void *spec, int32_t spec_len, char *buf, int32_t cap, bs_val *root) {
    bs_it_call c = {BS_IT_LOOT_ENCHANT, (const char *)spec, spec_len, 0, 0, 0, 0};
    return spec_len < 0 ? BASALT_ERR_INVALID : bs_it_value(&c, buf, cap, root);
}

/* The trades a villager of `profession` offers at `level` (1 to 5); `spec` may be NULL. */
static inline int32_t bs_trades_roll(const char *profession, int32_t level, const void *spec, int32_t spec_len,
                                     char *buf, int32_t cap, bs_val *root) {
    int32_t len = spec_len;
    const char *doc = bs_it_doc(spec, &len);
    bs_it_call c = {BS_IT_TRADES_ROLL, profession, bs_text_len(profession), doc, len, level, 0};
    return len < 0 ? BASALT_ERR_INVALID : bs_it_value(&c, buf, cap, root);
}

/* Registry data by kind ("item", "items", "enchantment", "tags", ...) as a value; `key` may be "". */
static inline int32_t bs_registry_get(const char *kind, const char *key, char *buf, int32_t cap, bs_val *root) {
    bs_it_call c = {BS_IT_REGISTRY, kind, bs_text_len(kind), key, bs_text_len(key), 0, 0};
    return bs_it_value(&c, buf, cap, root);
}

/* Evaluates a level based number (a bare number or a map of the datapack format) at `level`. */
static inline double bs_enchantment_level_value(const void *spec, int32_t spec_len, int32_t level) {
    return spec_len < 0 ? 0.0 : basalt_enchantment_level_value((const char *)spec, spec_len, level);
}

#endif
