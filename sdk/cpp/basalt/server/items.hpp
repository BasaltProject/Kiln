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

} // namespace basalt::server::items
