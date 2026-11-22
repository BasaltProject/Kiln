package org.basaltmc.server.items;
import org.basaltmc.server.internal.*;
import org.basaltmc.server.values.Values;
import org.basaltmc.server.core.*;

import static org.basaltmc.server.internal.Native.at;
import static org.basaltmc.server.internal.Native.utf8;


import org.teavm.interop.Address;

/** Items area: item stack documents, inventory handles, recipes, windows, container menus, loot and trades. */
public final class Items {
    private Items() {}

    private static final int ITEM_CAP = 512;
    private static final byte[] NONE = new byte[0];

    private static byte[] orEmpty(byte[] data) {
        return data == null ? NONE : data;
    }

    private interface Fill {
        int call(Address out, int cap);
    }

    private static int fill(byte[] out, Fill call) {
        return call.call(at(out), out.length);
    }

    /** The `{id, count[, name]}` document of a stack, or null when it does not encode; `name` may be null. */
    public static byte[] item(String id, int count, String name) {
        Values.Writer w = new Values.Writer().beginMap().key("id").string(id).key("count").integer(count);
        if (name != null) {
            w.key("name").string(name);
        }
        return w.end().finish();
    }

    /** A handle on a player, block, ender chest, window or entity inventory; release it when done. */
    public static final class Inventory {
        private int handle;

        private Inventory(int handle) {
            this.handle = handle;
        }

        public static Inventory ofPlayer(int player) { return new Inventory(RawItems.rawInventoryOfPlayer(player)); }
        public static Inventory ofWindow(int player) { return new Inventory(RawItems.rawInventoryOfWindow(player)); }
        public static Inventory ofEnder(int player) { return new Inventory(RawItems.rawInventoryOfEnder(player)); }
        public static Inventory ofEntity(int entity) { return new Inventory(RawItems.rawInventoryOfEntity(entity)); }

        public static Inventory ofBlock(String world, int x, int y, int z) {
            byte[] w = utf8(world);
            return new Inventory(RawItems.rawInventoryOfBlock(at(w), w.length, x, y, z));
        }

        /** The handle, or the negative error that made it. */
        public int handle() { return handle; }

        public boolean valid() { return handle > 0; }

        public int size() { return RawItems.rawInventorySize(handle); }

        public int heldSlot() { return RawItems.rawInventoryHeldSlot(handle); }

        public int setHeldSlot(int slot) { return RawItems.rawInventorySetHeldSlot(handle, slot); }

        public int put(int slot, byte[] item) {
            byte[] d = orEmpty(item);
            return RawItems.rawInventorySet(handle, slot, at(d), d.length);
        }

        public int put(int slot, String id, int count) { return put(slot, item(id, count, null)); }

        public int clearSlot(int slot) { return put(slot, new byte[] {0}); }

        /** Returns how many items did not fit. */
        public int give(byte[] item) {
            byte[] d = orEmpty(item);
            return RawItems.rawInventoryAdd(handle, at(d), d.length);
        }

        public int give(String id, int count, String name) { return give(item(id, count, name)); }

        public int take(String id, int count) {
            byte[] i = utf8(id);
            return RawItems.rawInventoryRemove(handle, at(i), i.length, count);
        }

        public int count(String id) {
            byte[] i = utf8(id);
            return RawItems.rawInventoryCount(handle, at(i), i.length);
        }

        /** The count of the stack in `slot`, 0 for an empty slot, negative for an error. */
        public int slotCount(int slot) {
            byte[] out = new byte[ITEM_CAP];
            int n = fill(out, (a, cap) -> RawItems.rawInventoryGet(handle, slot, a, cap));
            if (n < 0) {
                return n;
            }
            Values.Value root = n > ITEM_CAP ? null : Values.Value.open(java.util.Arrays.copyOf(out, n));
            if (root == null) {
                return Values.INVALID;
            }
            return root.isNull() ? 0 : (int) root.find("count").asLong(0);
        }

        public int clear() { return RawItems.rawInventoryClear(handle); }

        /** Fills a buffer of `cap` bytes with the slot document; the result is the length it needs. */
        public int get(int slot, int cap) {
            return fill(new byte[cap], (a, c) -> RawItems.rawInventoryGet(handle, slot, a, c));
        }

        public int contents(int cap) {
            return fill(new byte[cap], (a, c) -> RawItems.rawInventoryContents(handle, a, c));
        }

        public void release() {
            if (handle > 0) {
                RawItems.rawInventoryRelease(handle);
            }
            handle = 0;
        }
    }

    /** Puts an item in a container menu slot; `onClick` runs when a player clicks it and may be null. */
    public static int menuSlot(int menu, int slot, String id, int count, String name, Handler onClick) {
        byte[] d = item(id, count, name);
        if (d == null) {
            return Values.INVALID;
        }
        int token = onClick == null ? 0 : Callbacks.tokenFor(onClick);
        return RawItems.rawContainerMenuSet(menu, slot, at(d), d.length, token);
    }

    public static int itemCooldown(int player, String id) {
        byte[] i = utf8(id);
        return RawItems.rawItemCooldown(player, at(i), i.length);
    }

    public static int itemSetCooldown(int player, String id, int ticks) {
        byte[] i = utf8(id);
        return RawItems.rawItemSetCooldown(player, at(i), i.length, ticks);
    }

    public static boolean recipeKnown(String id) {
        byte[] i = utf8(id);
        return RawItems.rawRecipeHas(at(i), i.length) > 0;
    }

    /** Writes the registry entry into a buffer of `cap` bytes; the result is the length it needs. */
    public static int registryGet(String kind, String key, int cap) {
        byte[] k = utf8(kind);
        byte[] e = utf8(key);
        return fill(new byte[cap], (a, c) -> RawItems.rawRegistryGet(at(k), k.length, at(e), e.length, a, c));
    }

    public static double enchantmentLevelValue(byte[] spec, int level) {
        byte[] s = orEmpty(spec);
        return RawItems.rawEnchantmentLevelValue(at(s), s.length, level);
    }

    public static int recipeAdd(String id, String json) {
        byte[] i = utf8(id);
        byte[] j = utf8(json);
        return RawItems.rawRecipeAdd(at(i), i.length, at(j), j.length);
    }

    public static int recipeRemove(String id) {
        byte[] i = utf8(id);
        return RawItems.rawRecipeRemove(at(i), i.length);
    }

    public static int recipeIds(int cap) { return fill(new byte[cap], RawItems::rawRecipeIds); }

    public static int recipeGet(String id, int cap) {
        byte[] i = utf8(id);
        return fill(new byte[cap], (a, c) -> RawItems.rawRecipeGet(at(i), i.length, a, c));
    }

    public static int recipeClear() { return RawItems.rawRecipeClear(); }

    public static int recipeReset() { return RawItems.rawRecipeReset(); }

    public static int recipeResetBrewing() { return RawItems.rawRecipeResetBrewing(); }

    /** Runs a station recipe; the answer document is returned in `result` when it is not null. */
    public static int recipeStation(String kind, byte[] input, int cap, Values.OutResult[] result) {
        byte[] k = utf8(kind);
        byte[] in = orEmpty(input);
        Values.OutResult r = Values.callOut(cap,
                (a, c) -> RawItems.rawRecipeStation(at(k), k.length, at(in), in.length, a, c));
        if (result != null) {
            result[0] = r;
        }
        return r.error;
    }

    public static int recipeBookGet(int player, int cap) {
        return fill(new byte[cap], (a, c) -> RawItems.rawRecipeBookGet(player, a, c));
    }

    public static int recipeBookSet(int player, byte[] ids, boolean learn) {
        byte[] d = orEmpty(ids);
        return RawItems.rawRecipeBookSet(player, at(d), d.length, learn ? 1 : 0);
    }

    public static int recipeBookHas(int player, String id) {
        byte[] i = utf8(id);
        return RawItems.rawRecipeBookHas(player, at(i), i.length);
    }

    public static int recipeBookResend(int player) { return RawItems.rawRecipeBookResend(player); }

    public static int merchantOpen(int player, byte[] spec) {
        byte[] d = orEmpty(spec);
        return RawItems.rawMerchantOpen(player, at(d), d.length);
    }

    public static int windowOpen(int player, byte[] spec) {
        byte[] d = orEmpty(spec);
        return RawItems.rawWindowOpen(player, at(d), d.length);
    }

    public static int windowInfo(int player, int cap) {
        return fill(new byte[cap], (a, c) -> RawItems.rawWindowInfo(player, a, c));
    }

    public static int windowClose(int player) { return RawItems.rawWindowClose(player); }

    public static int windowUpdate(int player, byte[] spec) {
        byte[] d = orEmpty(spec);
        return RawItems.rawWindowUpdate(player, at(d), d.length);
    }

    public static int containerMenuCreate(byte[] spec) {
        byte[] d = orEmpty(spec);
        return RawItems.rawContainerMenuCreate(at(d), d.length);
    }

    public static int containerMenuClear(int menu) { return RawItems.rawContainerMenuClear(menu); }

    public static int containerMenuOpen(int menu, int player) { return RawItems.rawContainerMenuOpen(menu, player); }

    public static int containerMenuClose(int menu, int player) { return RawItems.rawContainerMenuClose(menu, player); }

    /** The click document of a menu handler, or null when it did not fit `cap` bytes. */
    public static Values.Value containerMenuClickInfo(int ctx, int cap) {
        byte[] out = new byte[cap];
        int n = fill(out, (a, c) -> RawItems.rawContainerMenuClickInfo(ctx, a, c));
        return n > 0 && n <= cap ? Values.Value.open(java.util.Arrays.copyOf(out, n)) : null;
    }

    public static int containerMenuClickResult(int ctx, byte[] item) {
        byte[] d = orEmpty(item);
        return RawItems.rawContainerMenuClickResult(ctx, at(d), d.length);
    }

    public static int containerMenuClickCancel(int ctx, boolean cancelled) {
        return RawItems.rawContainerMenuClickCancel(ctx, cancelled ? 1 : 0);
    }

    public static int lootTables(int cap) { return fill(new byte[cap], RawItems::rawLootTables); }

    public static int lootRoll(String table, byte[] spec, int cap) {
        byte[] t = utf8(table);
        byte[] s = orEmpty(spec);
        return fill(new byte[cap], (a, c) -> RawItems.rawLootRoll(at(t), t.length, at(s), s.length, a, c));
    }

    public static int lootRollJson(String json, byte[] spec, int cap) {
        byte[] j = utf8(json);
        byte[] s = orEmpty(spec);
        return fill(new byte[cap], (a, c) -> RawItems.rawLootRollJson(at(j), j.length, at(s), s.length, a, c));
    }

    public static int lootNumber(String json, boolean asFloat, long seed, int cap) {
        byte[] j = utf8(json);
        return fill(new byte[cap], (a, c) -> RawItems.rawLootNumber(at(j), j.length, asFloat ? 1 : 0, seed, a, c));
    }

    public static int lootApply(byte[] spec, int cap) {
        byte[] s = orEmpty(spec);
        return fill(new byte[cap], (a, c) -> RawItems.rawLootApply(at(s), s.length, a, c));
    }

    public static int lootEnchant(byte[] spec, int cap) {
        byte[] s = orEmpty(spec);
        return fill(new byte[cap], (a, c) -> RawItems.rawLootEnchant(at(s), s.length, a, c));
    }

    public static int tradesRoll(String profession, int level, byte[] spec, int cap) {
        byte[] p = utf8(profession);
        byte[] s = orEmpty(spec);
        return fill(new byte[cap],
                (a, c) -> RawItems.rawTradesRoll(at(p), p.length, level, at(s), s.length, a, c));
    }
}
