using Basalt.Server.Core;
using Basalt.Server.Internal;
using Basalt.Server.Values;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using static Basalt.Server.Core.Level;
using static Basalt.Server.Core.Priority;
using static Basalt.Server.Internal.Native;

namespace Basalt.Server.Items;

// Items area: an item stack builder, an Inventory over the inventory handles and thin wrappers for the other imports.
// Imports that answer a document fill a caller byte[] and return the length they need, as in the ABI.

/// <summary>An item stack document ready to pass to any import that takes a stack.</summary>
public sealed class Item
{
    public byte[] Bytes { get; }

    public bool Ok => Bytes != null;

    public int Length => Bytes == null ? -1 : Bytes.Length;

    public Item(string id, int count = 1, string name = null)
    {
        ValueWriter w = new ValueWriter(512);
        w.BeginMap().Key("id").String(id).Key("count").Int(count);
        if (name != null) w.Key("name").String(name);
        w.End();
        Bytes = w.Finish();
    }
}

/// <summary>A handle on a player, block, ender chest, window or entity inventory.</summary>
public sealed unsafe class Inventory
{
    int handle;

    Inventory(int handle) => this.handle = handle;

    public static Inventory OfPlayer(int player) => new Inventory(RawItems.InventoryOfPlayer(player));

    public static Inventory OfWindow(int player) => new Inventory(RawItems.InventoryOfWindow(player));

    public static Inventory OfEnder(int player) => new Inventory(RawItems.InventoryOfEnder(player));

    public static Inventory OfEntity(int entity) => new Inventory(RawItems.InventoryOfEntity(entity));

    public static Inventory OfBlock(string world, int x, int y, int z)
    {
        byte[] w = Encoding.UTF8.GetBytes(world);
        fixed (byte* p = w) return new Inventory(RawItems.InventoryOfBlock(p, w.Length, x, y, z));
    }

    /// <summary>The handle, or the negative error that made it.</summary>
    public int Handle => handle;

    public bool Valid => handle > 0;

    public int Size() => RawItems.InventorySize(handle);

    public int HeldSlot() => RawItems.InventoryHeldSlot(handle);

    public int SetHeldSlot(int slot) => RawItems.InventorySetHeldSlot(handle, slot);

    public int Put(int slot, Item item)
    {
        if (!item.Ok) return Catalog.ErrInvalid;
        fixed (byte* p = item.Bytes) return RawItems.InventorySet(handle, slot, p, item.Length);
    }

    public int Put(int slot, string id, int count) => Put(slot, new Item(id, count));

    public int ClearSlot(int slot)
    {
        byte empty = 0;
        return RawItems.InventorySet(handle, slot, &empty, 1);
    }

    /// <summary>Returns how many items did not fit.</summary>
    public int Give(Item item)
    {
        if (!item.Ok) return Catalog.ErrInvalid;
        fixed (byte* p = item.Bytes) return RawItems.InventoryAdd(handle, p, item.Length);
    }

    public int Give(string id, int count, string name = null) => Give(new Item(id, count, name));

    public int Take(string id, int count)
    {
        byte[] b = Encoding.UTF8.GetBytes(id);
        fixed (byte* p = b) return RawItems.InventoryRemove(handle, p, b.Length, count);
    }

    public int Count(string id)
    {
        byte[] b = Encoding.UTF8.GetBytes(id);
        fixed (byte* p = b) return RawItems.InventoryCount(handle, p, b.Length);
    }

    /// <summary>The count of the stack in the slot, 0 for an empty slot, negative for an error.</summary>
    public int SlotCount(int slot)
    {
        byte[] buffer = new byte[512];
        int n = Get(slot, buffer);
        if (n < 0) return n;
        if (n > buffer.Length) return Catalog.ErrInvalid;
        byte[] doc = new byte[n];
        Array.Copy(buffer, doc, n);
        if (!Value.TryOpen(doc, out Value root)) return Catalog.ErrInvalid;
        long count = 0;
        if (!root.IsNull) root.Find("count").TryInt(out count);
        return (int)count;
    }

    public int Clear() => RawItems.InventoryClear(handle);

    /// <summary>Writes the slot document to <paramref name="output"/>; the result is the length it needs.</summary>
    public int Get(int slot, byte[] output)
    {
        fixed (byte* p = output) return RawItems.InventoryGet(handle, slot, p, output.Length);
    }

    public int Contents(byte[] output)
    {
        fixed (byte* p = output) return RawItems.InventoryContents(handle, p, output.Length);
    }

    public void Release()
    {
        if (handle > 0) RawItems.InventoryRelease(handle);
        handle = 0;
    }
}

public static unsafe class Catalog
{
    public const int ErrInvalid = -3;

    static byte[] Utf8(string text) => Encoding.UTF8.GetBytes(text);

    /// <summary>Puts an item in a container menu slot; <paramref name="onClick"/> runs when a player clicks it.</summary>
    public static int MenuSlot(int menu, int slot, string id, int count, string name, Handler onClick)
    {
        Item item = new Item(id, count, name);
        if (!item.Ok) return Catalog.ErrInvalid;
        int token = onClick == null ? 0 : Native.TokenFor(onClick);
        fixed (byte* p = item.Bytes) return RawItems.ContainerMenuSet(menu, slot, p, item.Length, token);
    }

    public static int ItemCooldown(int player, string id)
    {
        byte[] b = Utf8(id);
        fixed (byte* p = b) return RawItems.ItemCooldown(player, p, b.Length);
    }

    public static int ItemSetCooldown(int player, string id, int ticks)
    {
        byte[] b = Utf8(id);
        fixed (byte* p = b) return RawItems.ItemSetCooldown(player, p, b.Length, ticks);
    }

    public static bool RecipeKnown(string id)
    {
        byte[] b = Utf8(id);
        fixed (byte* p = b) return RawItems.RecipeHas(p, b.Length) > 0;
    }

    public static int RegistryGet(string kind, string key, byte[] output)
    {
        byte[] k = Utf8(kind);
        byte[] v = Utf8(key);
        fixed (byte* kp = k)
        fixed (byte* vp = v)
        fixed (byte* op = output)
            return RawItems.RegistryGet(kp, k.Length, vp, v.Length, op, output.Length);
    }

    public static double EnchantmentLevelValue(byte[] spec, int level)
    {
        fixed (byte* p = spec) return RawItems.EnchantmentLevelValue(p, spec.Length, level);
    }

    public static int RecipeAdd(string id, string json)
    {
        byte[] i = Utf8(id);
        byte[] j = Utf8(json);
        fixed (byte* ip = i)
        fixed (byte* jp = j)
            return RawItems.RecipeAdd(ip, i.Length, jp, j.Length);
    }

    public static int RecipeRemove(string id)
    {
        byte[] b = Utf8(id);
        fixed (byte* p = b) return RawItems.RecipeRemove(p, b.Length);
    }

    public static int RecipeIds(byte[] output)
    {
        fixed (byte* p = output) return RawItems.RecipeIds(p, output.Length);
    }

    public static int RecipeGet(string id, byte[] output)
    {
        byte[] b = Utf8(id);
        fixed (byte* p = b)
        fixed (byte* op = output)
            return RawItems.RecipeGet(p, b.Length, op, output.Length);
    }

    public static int RecipeStation(string kind, byte[] input, byte[] output)
    {
        byte[] k = Utf8(kind);
        fixed (byte* kp = k)
        fixed (byte* ip = input)
        fixed (byte* op = output)
            return RawItems.RecipeStation(kp, k.Length, ip, input.Length, op, output.Length);
    }

    public static int RecipeStation(string kind, byte[] input, byte* output, int cap)
    {
        byte[] k = Utf8(kind);
        fixed (byte* kp = k)
        fixed (byte* ip = input)
            return RawItems.RecipeStation(kp, k.Length, ip, input.Length, output, cap);
    }

    public static int RecipeBookGet(int player, byte[] output)
    {
        fixed (byte* p = output) return RawItems.RecipeBookGet(player, p, output.Length);
    }

    public static int RecipeBookSet(int player, byte[] ids, bool learn)
    {
        fixed (byte* p = ids) return RawItems.RecipeBookSet(player, p, ids.Length, learn ? 1 : 0);
    }

    public static int RecipeBookHas(int player, string id)
    {
        byte[] b = Utf8(id);
        fixed (byte* p = b) return RawItems.RecipeBookHas(player, p, b.Length);
    }

    public static int RecipeBookResend(int player) => RawItems.RecipeBookResend(player);

    static int WithSpec(byte[] spec, Func<IntPtr, int, int> call)
    {
        fixed (byte* p = spec) return call((IntPtr)p, spec.Length);
    }

    public static int MerchantOpen(int player, byte[] spec) =>
        WithSpec(spec, (p, n) => RawItems.MerchantOpen(player, (byte*)p, n));

    public static int WindowOpen(int player, byte[] spec) =>
        WithSpec(spec, (p, n) => RawItems.WindowOpen(player, (byte*)p, n));

    public static int WindowInfo(int player, byte[] output)
    {
        fixed (byte* p = output) return RawItems.WindowInfo(player, p, output.Length);
    }

    public static int WindowClose(int player) => RawItems.WindowClose(player);

    public static int WindowUpdate(int player, byte[] spec) =>
        WithSpec(spec, (p, n) => RawItems.WindowUpdate(player, (byte*)p, n));

    public static int ContainerMenuCreate(byte[] spec) =>
        WithSpec(spec, (p, n) => RawItems.ContainerMenuCreate((byte*)p, n));

    public static int ContainerMenuOpen(int menu, int player) => RawItems.ContainerMenuOpen(menu, player);

    public static int ContainerMenuClose(int menu, int player) => RawItems.ContainerMenuClose(menu, player);

    public static int ContainerMenuClickInfo(int ctx, byte[] output)
    {
        fixed (byte* p = output) return RawItems.ContainerMenuClickInfo(ctx, p, output.Length);
    }

    public static int ContainerMenuClickResult(int ctx, Item item)
    {
        fixed (byte* p = item.Bytes) return RawItems.ContainerMenuClickResult(ctx, p, item.Length);
    }

    /// <summary>Clears the clicked slot (the null value).</summary>
    public static int ContainerMenuClickResultNull(int ctx)
    {
        byte empty = 0;
        return RawItems.ContainerMenuClickResult(ctx, &empty, 1);
    }

    public static int ContainerMenuClickCancel(int ctx, bool cancelled) =>
        RawItems.ContainerMenuClickCancel(ctx, cancelled ? 1 : 0);

    public static int LootTables(byte[] output)
    {
        fixed (byte* p = output) return RawItems.LootTables(p, output.Length);
    }

    public static int LootRoll(string table, byte[] spec, byte[] output)
    {
        byte[] t = Utf8(table);
        fixed (byte* tp = t)
        fixed (byte* sp = spec)
        fixed (byte* op = output)
            return RawItems.LootRoll(tp, t.Length, sp, spec == null ? 0 : spec.Length, op, output.Length);
    }

    public static int LootRollJson(string json, byte[] spec, byte[] output)
    {
        byte[] j = Utf8(json);
        fixed (byte* jp = j)
        fixed (byte* sp = spec)
        fixed (byte* op = output)
            return RawItems.LootRollJson(jp, j.Length, sp, spec == null ? 0 : spec.Length, op, output.Length);
    }

    public static int LootNumber(string json, bool asFloat, long seed, byte[] output)
    {
        byte[] j = Utf8(json);
        fixed (byte* jp = j)
        fixed (byte* op = output)
            return RawItems.LootNumber(jp, j.Length, asFloat ? 1 : 0, seed, op, output.Length);
    }

    public static int LootApply(byte[] spec, byte[] output)
    {
        fixed (byte* sp = spec)
        fixed (byte* op = output)
            return RawItems.LootApply(sp, spec.Length, op, output.Length);
    }

    public static int LootEnchant(byte[] spec, byte[] output)
    {
        fixed (byte* sp = spec)
        fixed (byte* op = output)
            return RawItems.LootEnchant(sp, spec.Length, op, output.Length);
    }

    public static int TradesRoll(string profession, int level, byte[] spec, byte[] output)
    {
        byte[] pr = Utf8(profession);
        fixed (byte* pp = pr)
        fixed (byte* sp = spec)
        fixed (byte* op = output)
            return RawItems.TradesRoll(pp, pr.Length, level, sp, spec == null ? 0 : spec.Length, op, output.Length);
    }
}
