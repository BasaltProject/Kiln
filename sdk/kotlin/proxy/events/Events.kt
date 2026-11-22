@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class, kotlin.wasm.ExperimentalWasmInterop::class)

// Proxy events area (abi/areas/proxy_events.h): event fields as values, chat, command and kick decisions, plugin
// identity, config arrays and typed command arguments. The *Value calls return the raw encoded value for the values
// reader, or null on a negative code.
package org.basaltmc.proxy.events
import org.basaltmc.proxy.core.*
import org.basaltmc.proxy.internal.*
import org.basaltmc.proxy.values.*


import kotlin.wasm.unsafe.MemoryAllocator
import kotlin.wasm.unsafe.withScopedMemoryAllocator

private const val peTagFalse = 1
private const val peTagTrue = 2
private const val peTagInt = 3
private const val peTagString = 5

private inline fun peFetchValue(call: MemoryAllocator.(Int, Int) -> Int): ByteArray? {
    var cap = 256
    while (true) {
        val r: Any? = withScopedMemoryAllocator { a ->
            val out = a.allocate(cap)
            val n = a.call(out.address.toInt(), cap)
            if (n < 0) null else if (n > cap) n else ByteArray(n) { (out + it).loadByte() }
        }
        if (r == null) return null
        if (r is Int) cap = r else return r as ByteArray
    }
}

private fun peReadLeb(bytes: ByteArray, at: IntArray): Long {
    var result = 0L
    var shift = 0
    while (shift < 70 && at[0] < bytes.size) {
        val b = bytes[at[0]++].toInt() and 0xff
        result = result or ((b and 0x7f).toLong() shl shift)
        if (b and 0x80 == 0) return result
        shift += 7
    }
    return -1L
}

private fun peLeb(out: ArrayList<Byte>, first: Long) {
    var value = first
    do {
        val b = (value and 0x7f).toInt()
        value = value ushr 7
        out.add((if (value != 0L) b or 0x80 else b).toByte())
    } while (value != 0L)
}

/** Registers an observer of the finished event; a decider is registered with [on]. */
fun watch(event: String, handler: Handler): Int = withScopedMemoryAllocator { a ->
    val e = a.str(event)
    rawProxyEventWatch(e.ptr, e.len, tokenFor(handler))
}

/** The encoded value of a field; an empty name gives a map of every readable field. */
fun eventGetValue(ctx: Ctx, field: String): ByteArray? = peFetchValue { out, cap ->
    val f = str(field)
    rawProxyEventGet(ctx, f.ptr, f.len, out, cap)
}

fun eventGetStr(ctx: Ctx, field: String): String? {
    val bytes = eventGetValue(ctx, field) ?: return null
    if (bytes.size < 2 || bytes[0].toInt() != peTagString) return null
    val at = intArrayOf(1)
    val length = peReadLeb(bytes, at)
    if (length < 0 || at[0] + length > bytes.size) return null
    return bytes.decodeToString(at[0], at[0] + length.toInt())
}

fun eventGetI64(ctx: Ctx, field: String): Long? {
    val bytes = eventGetValue(ctx, field) ?: return null
    if (bytes.size < 2 || bytes[0].toInt() != peTagInt) return null
    val zigzag = peReadLeb(bytes, intArrayOf(1))
    return (zigzag ushr 1) xor -(zigzag and 1)
}

fun eventGetBool(ctx: Ctx, field: String): Boolean? {
    val bytes = eventGetValue(ctx, field) ?: return null
    if (bytes.size != 1 || (bytes[0].toInt() != peTagTrue && bytes[0].toInt() != peTagFalse)) return null
    return bytes[0].toInt() == peTagTrue
}

/** A list of maps (name, type, readable, writable), one per field of the event. */
fun eventFields(ctx: Ctx): ByteArray? = peFetchValue { out, cap -> rawProxyEventFields(ctx, out, cap) }

fun eventSetValue(ctx: Ctx, field: String, value: ByteArray): Int = withScopedMemoryAllocator { a ->
    val f = a.str(field)
    val v = a.bytes(value)
    rawProxyEventSet(ctx, f.ptr, f.len, v.ptr, v.len)
}

fun eventSetText(ctx: Ctx, field: String, text: String): Int {
    val data = text.encodeToByteArray()
    val out = ArrayList<Byte>()
    out.add(peTagString.toByte())
    peLeb(out, data.size.toLong())
    for (b in data) out.add(b)
    return eventSetValue(ctx, field, out.toByteArray())
}

fun eventSetInt(ctx: Ctx, field: String, number: Long): Int {
    val out = ArrayList<Byte>()
    out.add(peTagInt.toByte())
    peLeb(out, (number shl 1) xor (number shr 63))
    return eventSetValue(ctx, field, out.toByteArray())
}

fun eventSetFlag(ctx: Ctx, field: String, flag: Boolean): Int =
    eventSetValue(ctx, field, byteArrayOf((if (flag) peTagTrue else peTagFalse).toByte()))

/** Cancels a decided event and shows [reason] (MiniMessage, or a '@' lang key) to the player. */
fun eventDeny(ctx: Ctx, reason: String = ""): Int {
    if (reason.isNotEmpty()) {
        val shown = eventSetText(ctx, "reason", reason)
        if (shown < 0) return shown
    }
    return eventSetFlag(ctx, "cancelled", true)
}

/** [field] is "id", "name" or "version". */
fun pluginInfo(field: String): String? = fetch { out, cap ->
    val f = str(field)
    rawProxyPluginInfo(f.ptr, f.len, out, cap)
}

fun pluginManifest(): ByteArray? = peFetchValue { out, cap -> rawProxyPluginManifest(out, cap) }

/** The config.toml value at a dotted key (arrays are lists, tables are maps); an empty key is the whole file. */
fun configValue(key: String): ByteArray? = peFetchValue { out, cap ->
    val k = str(key)
    rawProxyConfigValue(k.ptr, k.len, out, cap)
}

fun commandArgF64(ctx: Ctx, name: String): Double? = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val out = a.allocate(8)
    if (rawProxyCommandArgF64(ctx, n.ptr, n.len, out.address.toInt()) == 0) Double.fromBits(out.loadLong()) else null
}

fun commandArgBool(ctx: Ctx, name: String): Boolean? = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val out = a.allocate(4)
    if (rawProxyCommandArgBool(ctx, n.ptr, n.len, out.address.toInt()) == 0) out.loadInt() != 0 else null
}

fun commandArgs(ctx: Ctx): ByteArray? = peFetchValue { out, cap -> rawProxyCommandArgs(ctx, out, cap) }
