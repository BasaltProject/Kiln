@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class, kotlin.wasm.ExperimentalWasmInterop::class)

// Events area for Kotlin: strings and byte arrays over the raw imports, documents as Value (null on a failed call).
package org.basaltmc.server.events
import org.basaltmc.server.core.*
import org.basaltmc.server.internal.*
import org.basaltmc.server.values.*


import kotlin.wasm.unsafe.MemoryAllocator
import kotlin.wasm.unsafe.withScopedMemoryAllocator

const val EVENT_POST = 1
const val EVENT_CANCELLED = 2

private fun eventsText(capacity: Int, call: MemoryAllocator.(Int, Int) -> Int): String? {
    val result = callOut(capacity, call)
    return if (result.error == 0) result.bytes.decodeToString() else null
}

private fun eventsValue(initial: Int, call: MemoryAllocator.(Int, Int) -> Int): Value? = callOut(initial, call).value()

/** Listens to `name`; `filters` is a finished map document or null. */
fun eventOnEx(name: String, priority: Int, flags: Int, filters: ByteArray?, handler: Handler): Int =
    withScopedMemoryAllocator { a ->
        val n = a.str(name)
        val f = a.bytes(filters ?: ByteArray(0))
        rawEventOnEx(n.ptr, n.len, priority, flags, f.ptr, f.len, tokenFor(handler))
    }

fun eventName(ctx: Ctx): String? = eventsText(64) { out, cap -> rawEventName(ctx, out, cap) }

fun eventGet(ctx: Ctx, field: String, initial: Int): Value? =
    eventsValue(initial) { out, cap -> val f = str(field); rawEventGet(ctx, f.ptr, f.len, out, cap) }

fun eventSet(ctx: Ctx, field: String, value: ByteArray): Int = withScopedMemoryAllocator { a ->
    val f = a.str(field)
    val v = a.bytes(value)
    rawEventSet(ctx, f.ptr, f.len, v.ptr, v.len)
}

fun eventFields(ctx: Ctx, initial: Int): Value? = eventsValue(initial) { out, cap -> rawEventFields(ctx, out, cap) }

fun eventPayload(ctx: Ctx, initial: Int): Value? = eventsValue(initial) { out, cap -> rawEventPayload(ctx, out, cap) }

fun eventDescribe(name: String, initial: Int): Value? =
    eventsValue(initial) { out, cap -> val n = str(name); rawEventDescribe(n.ptr, n.len, out, cap) }

fun eventNames(initial: Int): Value? = eventsValue(initial) { out, cap -> rawEventNames(out, cap) }

fun eventCancelled(ctx: Ctx): Int = rawEventCancelled(ctx)

fun eventCancelWith(ctx: Ctx, text: String, args: Args? = null): Int = withScopedMemoryAllocator { a ->
    val t = a.str(text)
    val p = a.args(args)
    rawEventCancelWith(ctx, t.ptr, t.len, p.ptr, p.len)
}

fun eventCancelMessage(ctx: Ctx): String? = eventsText(256) { out, cap -> rawEventCancelMessage(ctx, out, cap) }

fun eventIsPost(ctx: Ctx): Int = rawEventIsPost(ctx)

fun eventEmit(name: String, value: ByteArray): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val v = a.bytes(value)
    rawEventEmit(n.ptr, n.len, v.ptr, v.len)
}

fun playerByUuid(uuid: String): Player = withScopedMemoryAllocator { a -> val u = a.str(uuid); rawPlayerByUuid(u.ptr, u.len) }

fun playerUuid(player: Player): String? = eventsText(64) { out, cap -> rawPlayerUuid(player, out, cap) }

fun playerGet(player: Player, name: String, initial: Int): Value? =
    eventsValue(initial) { out, cap -> val n = str(name); rawPlayerGet(player, n.ptr, n.len, out, cap) }

fun playerSet(player: Player, name: String, value: ByteArray): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val v = a.bytes(value)
    rawPlayerSet(player, n.ptr, n.len, v.ptr, v.len)
}

/** Runs a named operation; `args` is a finished list document or null. */
fun playerCall(player: Player, op: String, args: ByteArray?, initial: Int): Value? = eventsValue(initial) { out, cap ->
    val o = str(op)
    val a = bytes(args ?: ByteArray(0))
    rawPlayerCall(player, o.ptr, o.len, a.ptr, a.len, out, cap)
}

fun playerDescribe(initial: Int): Value? = eventsValue(initial) { out, cap -> rawPlayerDescribe(out, cap) }

/** The health in half hearts, null when the call failed. */
fun playerHealth(player: Player): Double? = withScopedMemoryAllocator { a ->
    val out = a.allocate(8)
    val p = out.address.toInt()
    if (rawPlayerHealth(player, p) < 0) {
        null
    } else {
        var bits = 0L
        for (i in 7 downTo 0) bits = (bits shl 8) or ((out + i).loadByte().toLong() and 0xFF)
        Double.fromBits(bits)
    }
}

fun playerSetHealth(player: Player, health: Double): Int = rawPlayerSetHealth(player, health)

fun playerGamemode(player: Player): String? = eventsText(16) { out, cap -> rawPlayerGamemode(player, out, cap) }

fun playerSetGamemode(player: Player, mode: String): Int =
    withScopedMemoryAllocator { a -> val m = a.str(mode); rawPlayerSetGamemode(player, m.ptr, m.len) }

fun playerTeleport(player: Player, world: String, x: Double, y: Double, z: Double, yaw: Float, pitch: Float): Int =
    withScopedMemoryAllocator { a -> val w = a.str(world); rawPlayerTeleport(player, w.ptr, w.len, x, y, z, yaw, pitch) }

fun playerTitle(player: Player, title: String, subtitle: String, fadeIn: Int, stay: Int, fadeOut: Int, args: Args? = null): Int =
    withScopedMemoryAllocator { a ->
        val t = a.str(title)
        val s = a.str(subtitle)
        val p = a.args(args)
        rawPlayerTitle(player, t.ptr, t.len, s.ptr, s.len, fadeIn, stay, fadeOut, p.ptr, p.len)
    }

fun playerKick(player: Player, text: String, args: Args? = null): Int = withScopedMemoryAllocator { a ->
    val t = a.str(text)
    val p = a.args(args)
    rawPlayerKick(player, t.ptr, t.len, p.ptr, p.len)
}

fun playerEffect(player: Player, effect: String, ticks: Int, amplifier: Int): Int =
    withScopedMemoryAllocator { a -> val e = a.str(effect); rawPlayerEffect(player, e.ptr, e.len, ticks, amplifier) }

fun playerOpenInventory(player: Player, kind: String, spec: ByteArray): Int = withScopedMemoryAllocator { a ->
    val k = a.str(kind)
    val s = a.bytes(spec)
    rawPlayerOpenInventory(player, k.ptr, k.len, s.ptr, s.len)
}

fun playerCloseWindow(player: Player): Int = rawPlayerCloseWindow(player)

/** Shows a form (a finished document); the handler runs once with the answer, read with `completionValue`. */
fun formOpen(player: Player, form: ByteArray, handler: Handler): Int = withScopedMemoryAllocator { a ->
    val f = a.bytes(form)
    rawFormOpen(player, f.ptr, f.len, tokenFor(handler))
}

fun formClose(player: Player): Int = rawFormClose(player)

fun dialogShow(player: Player, dialog: String, handler: Handler): Int =
    withScopedMemoryAllocator { a -> val d = a.str(dialog); rawDialogShow(player, d.ptr, d.len, tokenFor(handler)) }

fun dialogClear(player: Player): Int = rawDialogClear(player)

fun completionValue(ctx: Ctx, initial: Int): Value? = eventsValue(initial) { out, cap -> rawValueResult(ctx, out, cap) }
