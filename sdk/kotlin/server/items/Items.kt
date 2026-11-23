@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class)

package org.basaltmc.server.items

import kotlin.wasm.unsafe.MemoryAllocator
import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.server.core.*
import org.basaltmc.server.internal.*
import org.basaltmc.server.values.*

// Items area: item stack documents, inventory handles, recipes, windows, container menus, loot and trades.



private const val ITEM_CAP = 512

private fun fill(cap: Int, call: MemoryAllocator.(Int, Int) -> Int): Int = withScopedMemoryAllocator { a ->
    val out = a.allocate(cap).address.toInt()
    a.call(out, cap)
}

private fun MemoryAllocator.doc(data: ByteArray?): Bytes = if (data == null) Bytes(0, 0) else bytes(data)

/** The `{id, count[, name]}` document of a stack, or null when it does not encode; `name` may be null. */
fun itemStack(id: String, count: Int, name: String? = null): ByteArray? {
    val w = ValueWriter().beginMap().key("id").string(id).key("count").int(count.toLong())
    if (name != null) w.key("name").string(name)
    return w.end().finish()
}

/** A handle on a player, block, ender chest, window or entity inventory; release it when done. */
class Inventory private constructor(handle: Int) {
    /** The handle, or the negative error that made it. */
    var handle = handle
        private set

    companion object {
        fun ofPlayer(player: Int) = Inventory(rawInventoryOfPlayer(player))
        fun ofWindow(player: Int) = Inventory(rawInventoryOfWindow(player))
        fun ofEnder(player: Int) = Inventory(rawInventoryOfEnder(player))
        fun ofEntity(entity: Int) = Inventory(rawInventoryOfEntity(entity))

        fun ofBlock(world: String, x: Int, y: Int, z: Int) = Inventory(withScopedMemoryAllocator { a ->
            val w = a.str(world)
            rawInventoryOfBlock(w.ptr, w.len, x, y, z)
        })
    }

    val valid: Boolean get() = handle > 0

    fun size(): Int = rawInventorySize(handle)

    fun heldSlot(): Int = rawInventoryHeldSlot(handle)

    fun setHeldSlot(slot: Int): Int = rawInventorySetHeldSlot(handle, slot)

    fun put(slot: Int, item: ByteArray?): Int = withScopedMemoryAllocator { a ->
        val d = a.doc(item)
        rawInventorySet(handle, slot, d.ptr, d.len)
    }

    fun put(slot: Int, id: String, count: Int): Int = put(slot, itemStack(id, count))

    fun clearSlot(slot: Int): Int = put(slot, byteArrayOf(0))

    /** Returns how many items did not fit. */
    fun give(item: ByteArray?): Int = withScopedMemoryAllocator { a ->
        val d = a.doc(item)
        rawInventoryAdd(handle, d.ptr, d.len)
    }

    fun give(id: String, count: Int, name: String? = null): Int = give(itemStack(id, count, name))

    fun take(id: String, count: Int): Int = withScopedMemoryAllocator { a ->
        val i = a.str(id)
        rawInventoryRemove(handle, i.ptr, i.len, count)
    }

    fun count(id: String): Int = withScopedMemoryAllocator { a ->
        val i = a.str(id)
        rawInventoryCount(handle, i.ptr, i.len)
    }

    /** The count of the stack in `slot`, 0 for an empty slot, negative for an error. */
    fun slotCount(slot: Int): Int {
        val answer = callOut(ITEM_CAP) { out, cap -> rawInventoryGet(handle, slot, out, cap) }
        if (answer.error == CALL_TOO_LARGE) return -3
        if (answer.error != 0) return answer.error
        val root = answer.value() ?: return -3
        if (root.isNull) return 0
        return (root.find("count")?.asLong() ?: 0L).toInt()
    }

    fun clear(): Int = rawInventoryClear(handle)

    /** Fills a buffer of `cap` bytes with the slot document; the result is the length it needs. */
    fun get(slot: Int, cap: Int): Int = fill(cap) { out, c -> rawInventoryGet(handle, slot, out, c) }

    fun contents(cap: Int): Int = fill(cap) { out, c -> rawInventoryContents(handle, out, c) }

    fun release() {
        if (handle > 0) rawInventoryRelease(handle)
        handle = 0
    }
}

/** Puts an item in a container menu slot; `onClick` runs when a player clicks it and may be null. */
fun menuSlot(menu: Int, slot: Int, id: String, count: Int, name: String?, onClick: Handler?): Int {
    val item = itemStack(id, count, name) ?: return -3
    val token = if (onClick == null) 0 else tokenFor(onClick)
    return withScopedMemoryAllocator { a ->
        val d = a.bytes(item)
        rawContainerMenuSet(menu, slot, d.ptr, d.len, token)
    }
}

fun itemCooldown(player: Int, id: String): Int = withScopedMemoryAllocator { a ->
    val i = a.str(id)
    rawItemCooldown(player, i.ptr, i.len)
}

fun itemSetCooldown(player: Int, id: String, ticks: Int): Int = withScopedMemoryAllocator { a ->
    val i = a.str(id)
    rawItemSetCooldown(player, i.ptr, i.len, ticks)
}

fun recipeKnown(id: String): Boolean = withScopedMemoryAllocator { a ->
    val i = a.str(id)
    rawRecipeHas(i.ptr, i.len) > 0
}

/** Writes the registry entry into a buffer of `cap` bytes; the result is the length it needs. */
fun registryGet(kind: String, key: String, cap: Int): Int = fill(cap) { out, c ->
    val k = str(kind)
    val e = str(key)
    rawRegistryGet(k.ptr, k.len, e.ptr, e.len, out, c)
}

fun enchantmentLevelValue(spec: ByteArray?, level: Int): Double = withScopedMemoryAllocator { a ->
    val s = a.doc(spec)
    rawEnchantmentLevelValue(s.ptr, s.len, level)
}

fun recipeAdd(id: String, json: String): Int = withScopedMemoryAllocator { a ->
    val i = a.str(id)
    val j = a.str(json)
    rawRecipeAdd(i.ptr, i.len, j.ptr, j.len)
}

fun recipeRemove(id: String): Int = withScopedMemoryAllocator { a ->
    val i = a.str(id)
    rawRecipeRemove(i.ptr, i.len)
}

fun recipeIds(cap: Int): Int = fill(cap) { out, c -> rawRecipeIds(out, c) }

fun recipeGet(id: String, cap: Int): Int = fill(cap) { out, c ->
    val i = str(id)
    rawRecipeGet(i.ptr, i.len, out, c)
}

fun recipeClear(): Int = rawRecipeClear()

fun recipeReset(): Int = rawRecipeReset()

fun recipeResetBrewing(): Int = rawRecipeResetBrewing()

/** Runs a station recipe; the answer document is in the result's `bytes`, and `error` is the import status. */
fun recipeStation(kind: String, input: ByteArray?, cap: Int): OutResult = callOut(cap) { out, c ->
    val k = str(kind)
    val d = doc(input)
    rawRecipeStation(k.ptr, k.len, d.ptr, d.len, out, c)
}

fun recipeBookGet(player: Int, cap: Int): Int = fill(cap) { out, c -> rawRecipeBookGet(player, out, c) }

fun recipeBookSet(player: Int, ids: ByteArray?, learn: Boolean): Int = withScopedMemoryAllocator { a ->
    val d = a.doc(ids)
    rawRecipeBookSet(player, d.ptr, d.len, if (learn) 1 else 0)
}

fun recipeBookHas(player: Int, id: String): Int = withScopedMemoryAllocator { a ->
    val i = a.str(id)
    rawRecipeBookHas(player, i.ptr, i.len)
}

fun recipeBookResend(player: Int): Int = rawRecipeBookResend(player)

fun merchantOpen(player: Int, spec: ByteArray?): Int = withScopedMemoryAllocator { a ->
    val d = a.doc(spec)
    rawMerchantOpen(player, d.ptr, d.len)
}

fun windowOpen(player: Int, spec: ByteArray?): Int = withScopedMemoryAllocator { a ->
    val d = a.doc(spec)
    rawWindowOpen(player, d.ptr, d.len)
}

fun windowInfo(player: Int, cap: Int): Int = fill(cap) { out, c -> rawWindowInfo(player, out, c) }

fun windowClose(player: Int): Int = rawWindowClose(player)

fun windowUpdate(player: Int, spec: ByteArray?): Int = withScopedMemoryAllocator { a ->
    val d = a.doc(spec)
    rawWindowUpdate(player, d.ptr, d.len)
}

fun containerMenuCreate(spec: ByteArray?): Int = withScopedMemoryAllocator { a ->
    val d = a.doc(spec)
    rawContainerMenuCreate(d.ptr, d.len)
}

fun containerMenuClear(menu: Int): Int = rawContainerMenuClear(menu)

fun containerMenuOpen(menu: Int, player: Int): Int = rawContainerMenuOpen(menu, player)

fun containerMenuClose(menu: Int, player: Int): Int = rawContainerMenuClose(menu, player)

/** The click document of a menu handler, or null when it did not fit `cap` bytes. */
fun containerMenuClickInfo(ctx: Ctx, cap: Int): Value? {
    var need = 0
    val answer = callOut(cap) { out, c -> rawContainerMenuClickInfo(ctx, out, c).also { need = it } }
    return if (answer.error == 0 && need in 1..cap) answer.value() else null
}

fun containerMenuClickResult(ctx: Ctx, item: ByteArray?): Int = withScopedMemoryAllocator { a ->
    val d = a.doc(item)
    rawContainerMenuClickResult(ctx, d.ptr, d.len)
}

fun containerMenuClickCancel(ctx: Ctx, cancelled: Boolean): Int = rawContainerMenuClickCancel(ctx, if (cancelled) 1 else 0)

fun lootTables(cap: Int): Int = fill(cap) { out, c -> rawLootTables(out, c) }

fun lootRoll(table: String, spec: ByteArray?, cap: Int): Int = fill(cap) { out, c ->
    val t = str(table)
    val s = doc(spec)
    rawLootRoll(t.ptr, t.len, s.ptr, s.len, out, c)
}

fun lootRollJson(json: String, spec: ByteArray?, cap: Int): Int = fill(cap) { out, c ->
    val j = str(json)
    val s = doc(spec)
    rawLootRollJson(j.ptr, j.len, s.ptr, s.len, out, c)
}

fun lootNumber(json: String, asFloat: Boolean, seed: Long, cap: Int): Int = fill(cap) { out, c ->
    val j = str(json)
    rawLootNumber(j.ptr, j.len, if (asFloat) 1 else 0, seed, out, c)
}

fun lootApply(spec: ByteArray?, cap: Int): Int = fill(cap) { out, c ->
    val s = doc(spec)
    rawLootApply(s.ptr, s.len, out, c)
}

fun lootEnchant(spec: ByteArray?, cap: Int): Int = fill(cap) { out, c ->
    val s = doc(spec)
    rawLootEnchant(s.ptr, s.len, out, c)
}

fun tradesRoll(profession: String, level: Int, spec: ByteArray?, cap: Int): Int = fill(cap) { out, c ->
    val p = str(profession)
    val s = doc(spec)
    rawTradesRoll(p.ptr, p.len, level, s.ptr, s.len, out, c)
}
