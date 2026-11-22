//! Items area: an item stack builder and an Inventory over the inventory handles. No heap.
const core = @import("core.zig");
const values = @import("basalt_shared").values;
pub const raw = @import("raw/items.zig").raw;

pub const item_buffer: usize = 512;
const error_invalid: i32 = -3;

fn len(text: []const u8) i32 {
    return @intCast(text.len);
}

fn cap(out: []u8) i32 {
    return @intCast(out.len);
}

/// An item stack document in inline storage, ready to pass to any import that takes a stack.
pub const Item = struct {
    storage: [item_buffer]u8 = undefined,
    length: i32 = 0,

    pub fn init(id: []const u8, count: i32, name: ?[]const u8) Item {
        var item = Item{};
        var writer = values.Writer.init(&item.storage);
        writer.beginMap();
        writer.key("id");
        writer.string(id);
        writer.key("count");
        writer.int(count);
        if (name) |text| {
            writer.key("name");
            writer.string(text);
        }
        writer.end();
        item.length = if (writer.finish()) |document| len(document) else |_| error_invalid;
        return item;
    }

    pub fn ok(self: *const Item) bool {
        return self.length >= 0;
    }

    pub fn bytes(self: *const Item) []const u8 {
        return self.storage[0..@intCast(@max(self.length, 0))];
    }
};

/// A handle on a player, block, ender chest, window or entity inventory; call `release` when done.
pub const Inventory = struct {
    handle: i32 = 0,

    pub fn ofPlayer(player: core.Player) Inventory {
        return .{ .handle = raw.inventory_of_player(player) };
    }

    pub fn ofWindow(player: core.Player) Inventory {
        return .{ .handle = raw.inventory_of_window(player) };
    }

    pub fn ofEnder(player: core.Player) Inventory {
        return .{ .handle = raw.inventory_of_ender(player) };
    }

    pub fn ofEntity(entity: i32) Inventory {
        return .{ .handle = raw.inventory_of_entity(entity) };
    }

    pub fn ofBlock(world: []const u8, x: i32, y: i32, z: i32) Inventory {
        return .{ .handle = raw.inventory_of_block(world.ptr, len(world), x, y, z) };
    }

    pub fn valid(self: Inventory) bool {
        return self.handle > 0;
    }

    pub fn size(self: Inventory) i32 {
        return raw.inventory_size(self.handle);
    }

    pub fn heldSlot(self: Inventory) i32 {
        return raw.inventory_held_slot(self.handle);
    }

    pub fn setHeldSlot(self: Inventory, slot: i32) i32 {
        return raw.inventory_set_held_slot(self.handle, slot);
    }

    pub fn putItem(self: Inventory, slot: i32, item: *const Item) i32 {
        if (!item.ok()) return item.length;
        const document = item.bytes();
        return raw.inventory_set(self.handle, slot, document.ptr, len(document));
    }

    pub fn put(self: Inventory, slot: i32, id: []const u8, amount: i32) i32 {
        const item = Item.init(id, amount, null);
        return self.putItem(slot, &item);
    }

    pub fn clearSlot(self: Inventory, slot: i32) i32 {
        const empty = [1]u8{0};
        return raw.inventory_set(self.handle, slot, &empty, 1);
    }

    /// Returns how many items did not fit.
    pub fn giveItem(self: Inventory, item: *const Item) i32 {
        if (!item.ok()) return item.length;
        const document = item.bytes();
        return raw.inventory_add(self.handle, document.ptr, len(document));
    }

    pub fn give(self: Inventory, id: []const u8, amount: i32, name: ?[]const u8) i32 {
        const item = Item.init(id, amount, name);
        return self.giveItem(&item);
    }

    pub fn take(self: Inventory, id: []const u8, amount: i32) i32 {
        return raw.inventory_remove(self.handle, id.ptr, len(id), amount);
    }

    pub fn count(self: Inventory, id: []const u8) i32 {
        return raw.inventory_count(self.handle, id.ptr, len(id));
    }

    /// The count of the stack in `slot`, 0 for an empty slot, negative for an error.
    pub fn slotCount(self: Inventory, slot: i32) i32 {
        var buffer: [item_buffer]u8 = undefined;
        const n = raw.inventory_get(self.handle, slot, &buffer, @intCast(item_buffer));
        if (n < 0) return n;
        if (n > item_buffer) return error_invalid;
        const root = values.Value.open(buffer[0..@intCast(n)]) orelse return error_invalid;
        if (root.isNull()) return 0;
        const found = root.find("count") orelse return 0;
        return @intCast(found.asInt() orelse 0);
    }

    pub fn clear(self: Inventory) i32 {
        return raw.inventory_clear(self.handle);
    }

    /// Writes the slot document to `out`; the result is the length it needs.
    pub fn get(self: Inventory, slot: i32, out: []u8) i32 {
        return raw.inventory_get(self.handle, slot, out.ptr, cap(out));
    }

    pub fn contents(self: Inventory, out: []u8) i32 {
        return raw.inventory_contents(self.handle, out.ptr, cap(out));
    }

    pub fn release(self: *Inventory) void {
        if (self.handle > 0) _ = raw.inventory_release(self.handle);
        self.handle = 0;
    }
};

/// Puts an item in a container menu slot; `on_click` runs when a player clicks it.
pub fn menuSlot(menu: i32, slot: i32, id: []const u8, count: i32, name: ?[]const u8, on_click: ?core.Handler) i32 {
    const item = Item.init(id, count, name);
    if (!item.ok()) return item.length;
    const token = core.tokenFor(on_click);
    if (token < 0) return error_invalid;
    const document = item.bytes();
    return raw.container_menu_set(menu, slot, document.ptr, len(document), token);
}

pub fn itemCooldown(player: core.Player, id: []const u8) i32 {
    return raw.item_cooldown(player, id.ptr, len(id));
}

pub fn itemSetCooldown(player: core.Player, id: []const u8, ticks: i32) i32 {
    return raw.item_set_cooldown(player, id.ptr, len(id), ticks);
}

pub fn recipeKnown(id: []const u8) bool {
    return raw.recipe_has(id.ptr, len(id)) > 0;
}

pub fn registryGet(kind: []const u8, key: []const u8, out: []u8) i32 {
    return raw.registry_get(kind.ptr, len(kind), key.ptr, len(key), out.ptr, cap(out));
}
