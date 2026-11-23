#include <basalt/server.h>
#include <basalt/server/core.hpp>
#include <basalt/server/items.h>
#include <basalt/server/values.hpp>
// Items area: an item stack builder and an Inventory over the inventory handles. No heap, no exceptions.
#pragma once

namespace basalt::server::items {
using namespace basalt::server::core;
using namespace basalt::server::values;

/// An item stack document, built into inline storage and ready to pass to any import that takes a stack.
class Item {
public:
    Item(const char* id, int count = 1, const char* name = nullptr) {
        length_ = bs_item_stack(storage_, kCapacity, id, count, name);
    }

    bool ok() const { return length_ >= 0; }

    const char* data() const { return reinterpret_cast<const char*>(storage_); }

    int length() const { return length_; }

private:
    static constexpr int kCapacity = BS_ITEM_BUFFER;
    unsigned char storage_[kCapacity];
    int length_ = 0;
};

/// A handle on a player, block, ender chest, window or entity inventory. Released when the object goes away.
class Inventory {
public:
    Inventory() = default;
    Inventory(const Inventory&) = delete;
    Inventory& operator=(const Inventory&) = delete;

    Inventory(Inventory&& other) : handle_(other.handle_) { other.handle_ = 0; }

    ~Inventory() { release(); }

    static Inventory of_player(Player player) { return Inventory{basalt_inventory_of_player(player)}; }

    static Inventory of_window(Player player) { return Inventory{basalt_inventory_of_window(player)}; }

    static Inventory of_ender(Player player) { return Inventory{basalt_inventory_of_ender(player)}; }

    static Inventory of_entity(int entity) { return Inventory{basalt_inventory_of_entity(entity)}; }

    static Inventory of_block(const char* world, int x, int y, int z) {
        return Inventory{basalt_inventory_of_block(world, bs_text_len(world), x, y, z)};
    }

    /// The handle, or the negative error that made it.
    int handle() const { return handle_; }

    bool valid() const { return handle_ > 0; }

    int size() const { return basalt_inventory_size(handle_); }

    int held_slot() const { return basalt_inventory_held_slot(handle_); }

    int set_held_slot(int slot) const { return basalt_inventory_set_held_slot(handle_, slot); }

    int put(int slot, const Item& item) const {
        return basalt_inventory_set(handle_, slot, item.data(), item.length());
    }

    int put(int slot, const char* id, int count) const { return bs_inventory_put(handle_, slot, id, count); }

    int clear_slot(int slot) const {
        const char empty[1] = {static_cast<char>(BASALT_VALUE_NULL)};
        return basalt_inventory_set(handle_, slot, empty, 1);
    }

    /// Returns how many items did not fit.
    int give(const Item& item) const { return basalt_inventory_add(handle_, item.data(), item.length()); }

    int give(const char* id, int count, const char* name = nullptr) const {
        return bs_inventory_give(handle_, id, count, name);
    }

    int take(const char* id, int count) const { return bs_inventory_take(handle_, id, count); }

    int count(const char* id) const { return bs_inventory_has(handle_, id); }

    int slot_count(int slot) const { return bs_inventory_slot_count(handle_, slot); }

    int clear() const { return basalt_inventory_clear(handle_); }

    /// Writes the slot document to `out`; the result is the length it needs.
    int get(int slot, char* out, int cap) const { return basalt_inventory_get(handle_, slot, out, cap); }

    int contents(char* out, int cap) const { return basalt_inventory_contents(handle_, out, cap); }

    void release() {
        if (handle_ > 0) {
            basalt_inventory_release(handle_);
        }
        handle_ = 0;
    }

private:
    explicit Inventory(int handle) : handle_(handle) {}

    int handle_ = 0;
};

/// Puts an item in a container menu slot; `on_click` runs when a player clicks it.
inline int menu_slot(int menu, int slot, const char* id, int count, const char* name, Handler on_click) {
    return bs_menu_slot(menu, slot, id, count, name, on_click);
}

inline int item_cooldown(Player player, const char* id) { return bs_cooldown(player, id); }

inline int item_set_cooldown(Player player, const char* id, int ticks) { return bs_set_cooldown(player, id, ticks); }

inline bool recipe_known(const char* id) { return bs_recipe_known(id) > 0; }

inline bool recipe_has(const char* id) { return bs_recipe_has(id) == 1; }

inline int recipe_add(const char* id, const char* json) { return bs_recipe_add(id, json); }

inline int recipe_remove(const char* id) { return bs_recipe_remove(id); }

/// Every recipe id, a list of strings; the list is long, so give the reply room.
template <int N>
inline int recipe_ids(Reply<N>& reply) {
    return values::detail::fill_reply(reply,
                                      [](char* out, int cap, bs_val* raw) { return bs_recipe_ids(out, cap, raw); });
}

inline int recipe_clear() { return bs_recipe_clear(); }

inline int recipe_reset() { return bs_recipe_reset(); }

inline int recipe_reset_brewing() { return bs_recipe_reset_brewing(); }

template <int N>
inline int recipe_get(const char* id, Reply<N>& reply) {
    return values::detail::fill_reply(
        reply, [&](char* out, int cap, bs_val* raw) { return bs_recipe_get(id, out, cap, raw); });
}

/// What a station makes: `kind` is "crafting", "anvil" or "smithing" and `input` its document.
template <int N>
inline int recipe_station(const char* kind, const ValueWriter& input, Reply<N>& reply) {
    return values::detail::fill_reply(reply, [&](char* out, int cap, bs_val* raw) {
        return bs_recipe_station(kind, input.data(), input.finish(), out, cap, raw);
    });
}

template <int N>
inline int recipe_book_get(Player player, Reply<N>& reply) {
    return values::detail::fill_reply(
        reply, [&](char* out, int cap, bs_val* raw) { return bs_recipe_book_get(player, out, cap, raw); });
}

/// `ids` is a list of recipe ids; returns how many changed.
inline int recipe_book_set(Player player, const ValueWriter& ids, bool learn = true) {
    return bs_recipe_book_set(player, ids.data(), ids.finish(), learn ? 1 : 0);
}

inline bool recipe_book_has(Player player, const char* id) { return bs_recipe_book_has(player, id) == 1; }

inline int recipe_book_resend(Player player) { return bs_recipe_book_resend(player); }

inline int merchant_open(Player player, const ValueWriter& spec) {
    return bs_merchant_open(player, spec.data(), spec.finish());
}

/// Returns the container id.
inline int window_open(Player player, const ValueWriter& spec) {
    return bs_window_open(player, spec.data(), spec.finish());
}

template <int N>
inline int window_info(Player player, Reply<N>& reply) {
    return values::detail::fill_reply(
        reply, [&](char* out, int cap, bs_val* raw) { return bs_window_info(player, out, cap, raw); });
}

inline int window_close(Player player) { return bs_window_close(player); }

inline int window_update(Player player, const ValueWriter& spec) {
    return bs_window_update(player, spec.data(), spec.finish());
}

inline int container_menu_create(const ValueWriter& spec) {
    return bs_container_menu_create(spec.data(), spec.finish());
}

/// Puts an item in a slot; `on_click` runs when a player clicks it.
inline int container_menu_set(int menu, int slot, const Item& item, Handler on_click = nullptr) {
    return bs_container_menu_set(menu, slot, item.data(), item.length(), on_click);
}

inline int container_menu_set(int menu, int slot, const ValueWriter& item, Handler on_click = nullptr) {
    return bs_container_menu_set(menu, slot, item.data(), item.finish(), on_click);
}

inline int container_menu_clear_slot(int menu, int slot) {
    return bs_container_menu_set(menu, slot, nullptr, 0, nullptr);
}

inline int container_menu_clear(int menu) { return bs_container_menu_clear(menu); }

inline int container_menu_open(int menu, Player player) { return bs_container_menu_open(menu, player); }

inline int container_menu_close(int menu, Player player) { return bs_container_menu_close(menu, player); }

/// Inside a click handler: {player, slot, button, mode}.
template <int N>
inline int container_menu_click_info(Ctx ctx, Reply<N>& reply) {
    return values::detail::fill_reply(
        reply, [&](char* out, int cap, bs_val* raw) { return bs_container_menu_click_info(ctx, out, cap, raw); });
}

/// Inside a click handler: replaces the stack shown in the clicked slot.
inline int container_menu_click_result(Ctx ctx, const Item& item) {
    return bs_container_menu_click_result(ctx, item.data(), item.length());
}

inline int container_menu_click_result(Ctx ctx, const ValueWriter& item) {
    return bs_container_menu_click_result(ctx, item.data(), item.finish());
}

inline int container_menu_click_clear(Ctx ctx) { return bs_container_menu_click_result(ctx, nullptr, 0); }

inline int container_menu_click_cancel(Ctx ctx, bool cancelled = true) {
    return bs_container_menu_click_cancel(ctx, cancelled ? 1 : 0);
}

namespace impl {
inline const char* data_of(const ValueWriter* doc) { return doc == nullptr ? nullptr : doc->data(); }

inline int length_of(const ValueWriter* doc) { return doc == nullptr ? 0 : doc->finish(); }
} // namespace impl

template <int N>
inline int loot_tables(Reply<N>& reply) {
    return values::detail::fill_reply(reply,
                                      [](char* out, int cap, bs_val* raw) { return bs_loot_tables(out, cap, raw); });
}

/// Rolls a loot table into a list of items; `spec` (world, x, y, z, luck, seed, looter) may be null.
template <int N>
inline int loot_roll(const char* table, const ValueWriter* spec, Reply<N>& reply) {
    return values::detail::fill_reply(reply, [&](char* out, int cap, bs_val* raw) {
        return bs_loot_roll(table, impl::data_of(spec), impl::length_of(spec), out, cap, raw);
    });
}

template <int N>
inline int loot_roll_json(const char* json, const ValueWriter* spec, Reply<N>& reply) {
    return values::detail::fill_reply(reply, [&](char* out, int cap, bs_val* raw) {
        return bs_loot_roll_json(json, impl::data_of(spec), impl::length_of(spec), out, cap, raw);
    });
}

/// Evaluates a number provider given as JSON text, an int or a float value.
template <int N>
inline int loot_number(const char* json, bool as_float, long long seed, Reply<N>& reply) {
    return values::detail::fill_reply(reply, [&](char* out, int cap, bs_val* raw) {
        return bs_loot_number(json, as_float ? 1 : 0, seed, out, cap, raw);
    });
}

template <int N>
inline int loot_apply(const ValueWriter& spec, Reply<N>& reply) {
    return values::detail::fill_reply(reply, [&](char* out, int cap, bs_val* raw) {
        return bs_loot_apply(spec.data(), spec.finish(), out, cap, raw);
    });
}

template <int N>
inline int loot_enchant(const ValueWriter& spec, Reply<N>& reply) {
    return values::detail::fill_reply(reply, [&](char* out, int cap, bs_val* raw) {
        return bs_loot_enchant(spec.data(), spec.finish(), out, cap, raw);
    });
}

/// The trades a villager of `profession` offers at `level` (1 to 5); `spec` may be null.
template <int N>
inline int trades_roll(const char* profession, int level, const ValueWriter* spec, Reply<N>& reply) {
    return values::detail::fill_reply(reply, [&](char* out, int cap, bs_val* raw) {
        return bs_trades_roll(profession, level, impl::data_of(spec), impl::length_of(spec), out, cap, raw);
    });
}

/// Registry data by kind ("item", "items", "enchantment", "tags", ...); `key` may be "".
template <int N>
inline int registry_get(const char* kind, const char* key, Reply<N>& reply) {
    return values::detail::fill_reply(
        reply, [&](char* out, int cap, bs_val* raw) { return bs_registry_get(kind, key, out, cap, raw); });
}

inline double enchantment_level_value(const ValueWriter& spec, int level) {
    return bs_enchantment_level_value(spec.data(), spec.finish(), level);
}

} // namespace basalt::server::items
